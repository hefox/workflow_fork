<?php
// $Id$
/**
 * CTool CRUD functions.
 */

/**
 * Provides CTools integration callbacks as if a faux-exportable.
 * CTools doesn't natively handle complex objects (like anything with a 1:Many relationship in tables.
 * Workflow has that all over the place, so we're going to use CRUD callbacks to bridge the gap.
 * From: http://drupalcontrib.org/api/drupal/contributions--ctools--includes--export.inc/group/export_crud/7
 *
 * We are tackling this workflow object as one huge monolithic object called workflow.
 * This object isn't used in any other code in the system. It's massive overkill.
 * Using the object properly thoughout the module means a full rewrite, and I don't want to go halfway on this.
 * I would rather use this object only in the context of a CRUD interaction at this stage of things.
 * These CRUD functions are just wrappers for our regular CRUD functions and just translate between
 * the two systems (regular code CRUD functions vs. CTools).
 *
 * Workflow is constructed as follows:
 *   workflow : properties matching the fields in table workflows
 *   workflow->states[] : an array of sub objects matching the fields in workflow_states
 *   workflow->transitions[] : an array of sub objects matching the fields in workflow_transitions
 *     ALERT: roles values in this are based on the role ids in site, roles lack machine names,
 *       and aren't exported, thought their names are unique and could be used to map between role ids.
 *       TODO: This should be done in round two of this integration!
 *   workflow->node_types[] : an array of sub objects mathing the fields found in workflow_type_map
 *     ALERT: node types are machine names, so if you're using this as part of a feature, you wanna export types.
 *
 * We are leaving some tables out on purpose: workflow_scheduled_transition, workflow_node_history, workflow_node.
 * Those are related directly to the content in a site and should be handled outside of this integration.
 */

/**
 * Implements 'delete callback' for workflow exportables.
 */
function workflow_workflow_delete($workflow) {
  // workflow_workflow_load makes sure we have the wid.
  workflow_delete_workflows_by_wid($workflow->wid);
}

// Our object doesn't disable / enable, so we don't use these functions.
// function workflow_workflow_disable($object)
// function workflow_workflow_enable($object)

/**
 * Implements 'export callback' for workflow exportables.
 */
function workflow_workflow_export($workflow, $indent = '') {
  ctools_include('export');
  $workflow = workflow_get_workflows_by_name_in_CRUD($workflow->name);
  // Anything not in our object def will be ignored on export. That's ok.
  $states = $workflow->states;
  $transitions = $workflow->transitions;
  $node_types = $workflow->node_types;
  unset($workflow->wid);
  unset($workflow->states);
  unset($workflow->transitions);
  unset($workflow->node_types);
  $export = ctools_export_object('workflows', $workflow, $indent);
  // Now we need to add data to the export for each state, transition, and node_map as arrays of sub-objects.
  // Since states are unique to workflows (not shared), we could safely pull state data sans sid.
  // BUT. Transitions need to know state sid to trace which state they belong to. (and therefore which wid).
  // Without changing any tables, state->state is a unique value we can use instead of sid.
  // The code up till now has enforced this as unique, so we're adding the unique restraint on it and moving forward.
  // (Same applies to workflow->name for that matter).
  // On save we're going to want new sids so we need to relate state name to state sid here.
  foreach ($states as $index => $state) {
    unset($state->sid);
    unset($state->wid);
    // Converting to an array here, because there's a bug in CTools at ctools_var_export(). It bugs out on objects due to a stray .= .
    $export .= $indent . '$workflow->states[' . $index . '] = (object) ' . ctools_var_export((array) $state, '') . ";\n";
  }
  // Now we need to add data to the export for each transition, an array of sub-objects.
  // Same goes for transitions, see above re: states.
  foreach ($transitions as $index => $transition) {
    // We'll preserve the state name data, and drop serial values.
    unset($transition->tid);
    unset($transition->sid);
    unset($transition->target_sid);
    // Converting to an array here, because there's a bug in CTools at ctools_var_export(). It bugs out on objects due to a stray .= .
    $export .= $indent . '$workflow->transitions[' . $index . '] = (object) ' . ctools_var_export((array) $transition, '') . ";\n";
  }
  // Now we need to add data to the export for each type map, an array of sub-objects.
  foreach ($node_types as $index => $type_map) {
    unset($type_map->wid);
    // Converting to an array here, because there's a bug in CTools at ctools_var_export(). It bugs out on objects due to a stray .= .
    $export .= $indent . '$workflow->node_types[' . $index . '] = (object) ' . ctools_var_export((array) $type_map, '') . ";\n";
  }
  return $export;
}

// Our object uses the default, which takes the code and just reads it, returning it to be saved.
// function workflow_workflow_import($code)

/**
 * Implements 'load callback' for workflow exportables.
 */
function workflow_workflow_load($name) {
  $workflow = workflow_get_workflows_by_name_in_CRUD($name);
  // Note, this also returns a lot of extrenous wid, sid, tid, etc data that our later funcs don't need.
  // That's ok at this stage of things, it's just data we'll ignore them on export.
  return $return[$name] = $workflow;
}

/**
 * Implements 'load all callback' for workflow exportables.
 */
function workflow_workflow_load_all($reset) {
  // Reset is meant to flush caches, occurs before we're called here.
  $return = array();
  foreach (workflow_get_workflows() as $workflow) {
    $return[$workflow->name] = workflow_get_workflows_by_name_in_CRUD($workflow->name);
  }
  return $return;
}

// Our object uses the default, which takes the schema table and creates an empty cong, no keywords, without saving.
// To save keywords, we must go to our customized admin UI, or add keywords before saving.
// function workflow_workflow_new($set_defaults)

/**
 * Implements 'save callback' for workflow exportables.
 */
function workflow_workflow_save($workflow) {
  // Given a workflow in the format returned from export.
  // First we grab the states, transitions and node_maps out.
  $states = $workflow->states;
  unset($workflow->states);
  $transitions = $workflow->transitions;
  unset($workflow->transitions);
  $node_types = $workflow->node_types;
  unset($workflow->node_types);
  // Then make a workflow so we can track by wid.
  $workflow = (array) $workflow;
  workflow_update_workflows($workflow, FALSE);
  if (!isset($workflow->wid) || empty($workflow->wid)) {
    return FALSE;
  }
  // Workflow is now a fully vetted workflow object. We have NOT created a creation state with this.
  // Then make states, marking state name to state sid.
  $active_states = array();
  foreach ($states as $state) {
    $state->wid = $workflow->wid;
    $state = (array) $state;
    workflow_update_workflow_states($state);
    $active_states[$state->name] = $state->sid;
  }
  // Then make transitions with the state mapping.
  foreach ($transitions as $transition) {
    $transition->sid = $active_states[$transition->sid_state];
    $transition->target_sid = $active_states[$transition->target_sid_state];
    unset($transition->sid_state);
    unset($transition->target_sid_state);
    $transition = (array) $transition;
    // NOTE: There is an assumption here with roles, that the rids are the same between installs.
    // This is rather unavoidable at current configs. SO: either roles need to be exported too, or
    // The installers should check role states with additional code.
    workflow_update_workflow_transitions($transition);
  }
  // Then add the node_type mapping.
  foreach ($node_types as $node_type) {
    $node_type->wid = $workflow->wid;
    $node_type = (array) $node_type;
    // Insert, nodes only have one workflow. Insert will delete any prior workflow assoc.
    workflow_insert_workflow_type_map($node_type);
  }
  return SAVED_NEW;
}

// This is called by the enable / disable code. We don't enable / disable this object.
// function workflow_workflow_set_status($object, $status)


/**
 * For use by CRUD only, gather everythign into the CRUD formed object.
 */
function workflow_get_workflows_by_name_in_CRUD($name) {
  $workflow = workflow_get_workflows_by_name($name);
  // Now we need to add data to the object for each state, an array of sub-objects.
  $options = array('status' => 1); // We only want active states for this export.
  $active_states = array();
  foreach (workflow_get_workflow_states_by_wid($workflow->wid, $options) as $index => $state) {
    $active_states[$state->sid] = $state->state;
    $workflow->states[] = $state;
  }
  // Now we need to add data to the export for each transition, an array of sub-objects.
  // Same goes for transitions, see above re: states.
  foreach ($active_states as $sid => $state) {
    // We're going to look everythign up by the start state, not state involved, to avoid dupes.
    foreach (workflow_get_workflow_transitions_by_sid($sid, $options) as $transition) {
      // And to get the target state (by name) we need to look it up too.
      $target_state = workflow_get_workflow_states_by_sid($transition->target_sid);
      $transition->sid_state = $state;
      $transition->target_sid_state = $target_state->state;
      $workflow->transitions[] = $transition;
    }
  }
  // Now we need to add data to the export for each type map, an array of sub-objects.
  // Same goes for node mappings, see above re: states.
  foreach (workflow_get_workflow_type_map_by_wid($workflow->wid) as $index => $type_map) {
    $workflow->node_types[] = $type_map;
  }
  return $workflow;
}

/**
 * CRUD DB specific functions.
 */

/**
 * Get a specific workflow, name is a unique ID.
 */
function workflow_get_workflows_by_name($name) {
  $results = db_query('SELECT * FROM {workflows} WHERE name = :name', array(':name' => $name));
  if ($workflow = $results->fetchObject()) {
    // This is only called by CRUD functions in workflow.ctools.inc
    // More than likely in prep for an import / export action.
    // Therefore we don't want to fiddle with the response.
    // $workflow->options = unserialize($workflow->options);
    return $workflow;
  }
  return FALSE;
}