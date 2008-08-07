<?php
// $Id$
/**
 * @file
 * Provide views data and handlers for workflow.module.
 */

/**
 * @defgroup views_workflow_module workflow.module handlers
 *
 * Includes the 'workflow_node' and 'workflow_state' tables, but generates a
 * pseudo-table for each separate workflow that's been set up.
 * @{
 */

/**
 * Implementation of hook_views_data().
 */
function workflow_views_data() {
  $data['workflow_states']['table']['group']  = t('Workflow');
  // Explain how this table joins to others.
  $data['workflow_states']['table']['join'] = array(
    'node' => array(
      'field' => 'sid',
      'left_table' => 'workflow_node',
      'left_field' => 'sid',
    ),
  );

  // state weight
  $data['workflow_states']['weight'] = array(
    'title' => t('State weight'), // Appears in views UI.
    'help' => t('The weight of the current workflow state that the node is in.'),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric',
    ),
  );

  // state
  $data['workflow_states']['state'] = array(
    'title' => t('Current state name'), // Appears in views UI.
    'help' => t('The readable name of the workflow state that the node is in. (Less efficient, use only when click-sorting by state name.)'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
     ),
  );

  $data['workflow_node']['table']['group']  = t('Workflow');
  // Explain how this table joins to others.
  $data['workflow_node']['table']['join'] = array(
    'node' => array(
      'field' => 'nid',
      'left_table' => 'node',
      'left_field' => 'nid',
    ),
  );

  // state
  $data['workflow_node']['sid'] = array(
    'title' => t('Current state'), // Appears in views UI.
    'help' => t('The current workflow state that the node is in.'),
    'field' => array(
      'handler' => 'workflow_views_handler_field_sid',
      'click sortable' => TRUE,
     ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
      'click sortable' => TRUE,
      'numeric' => TRUE,
    ),
    'filter' => array(
      'handler' => 'workflow_views_handler_filter_sid',
      'numeric' => TRUE,
    ),
  );

  $data['workflow_scheduled_transition']['table']['group']  = t('Workflow');
  // Explain how this table joins to others.
  $data['workflow_scheduled_transition']['table']['join'] = array(
    'node' => array(
      'field' => 'nid',
      'left_table' => 'node',
      'left_field' => 'nid',
    ),
  );

  // state
  $data['workflow_scheduled_transition']['sid'] = array(
    'title' => t('Scheduled state'), // Appears in views UI.
    'help' => t('The current workflow state that the node is in.'),
    'field' => array(
      'handler' => 'workflow_views_handler_field_sid',
      'click sortable' => TRUE,
     ),
    'filter' => array(
      'handler' => 'workflow_views_handler_filter_sid',
      'numeric' => TRUE,
    ),
  );

  // time
  $data['workflow_scheduled_transition']['scheduled'] = array(
    'title' => t('Scheduled time'), // Appears in views UI.
    'help' => t('The time at which the node will change workflow states.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
     ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
      'numeric' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
  );

  // comment
  $data['workflow_scheduled_transition']['comment'] = array(
    'title' => t('Scheduled comment'), // Appears in views UI.
    'help' => t('A comment describing why the node was scheduled for state transition.'),
    'field' => array(
      'handler' => 'views_handler_field_xss',
      'click sortable' => TRUE,
     ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
      'numeric' => TRUE,
    ),
  );

  $data['workflow_node_history']['table']['group']  = t('Workflow');
  // Explain how this table joins to others.
  $data['workflow_node_history']['table']['join'] = array(
    'node' => array(
      'field' => 'nid',
      'left_table' => 'node',
      'left_field' => 'nid',
    ),
  );

  // state
  $data['workflow_node_history']['sid'] = array(
    'title' => t('Previous state'), // Appears in views UI.
    'help' => t('A workflow state that the node was in previously.'),
    'field' => array(
      'handler' => 'workflow_views_handler_field_sid',
      'click sortable' => TRUE,
     ),
    'filter' => array(
      'handler' => 'workflow_views_handler_filter_sid',
      'numeric' => TRUE,
    ),
  );

  // time
  $data['workflow_node_history']['stamp'] = array(
    'title' => t('Previous time'), // Appears in views UI.
    'help' => t('The time at which the node moved from one state to another.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
     ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
      'numeric' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
  );

  // comment
  $data['workflow_node_history']['comment'] = array(
    'title' => t('Previous comment'), // Appears in views UI.
    'help' => t('A comment describing why the node was moved from one state to another in the past.'),
    'field' => array(
      'handler' => 'views_handler_field_xss',
      'click sortable' => TRUE,
     ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
      'numeric' => TRUE,
    ),
  );

  return $data;
}


/**
 * Argument handler to accept a node type.
 */
class views_handler_argument_workflow_state extends views_handler_argument {
  function construct() {
    parent::construct('type');
  }

  /**
   * Override the behavior of summary_name(). Get the user-friendly version
   * of the workflow state.
   */
  function summary_name($data) {
    return $this->workflow_states($data->{$this->name_alias});
  }

  /**
   * Override the behavior of title(). Get the user-friendly version of the
   * workflow state.
   */
  function title() {
    return $this->workflow_states($this->argument);
  }

  function workflow_states($sid) {
    if (empty($sid)) {
      return t('No state');
    }

    static $states;
    if (!isset($states)) {
      $states = workflow_get_states();
    }
    $output = $states[$sid];
    if (empty($output)) {
      $output = t('No state');
    }
    return check_plain($output);
  }
}

/**
 * Filter by node type
 */
class workflow_views_handler_filter_sid extends views_handler_filter_in_operator {
  var $value_form_type = 'select';

  function get_value_options() {
    if (!isset($this->value_options)) {
      $this->value_title = t('Workflow state');

      $workflows = workflow_get_all();
      if (count($workflows) > 1) {
        $states = array('' => t('No state'));
        foreach ($workflows as $wid => $wname) {
          $states[$wname] = workflow_get_states($wid);
        }
      }
      else {
        $states = workflow_get_states();
      }

      $this->value_options = $states;
    }
  }

  function query() {
    if (empty($this->value)) {
      return;
    }
    $this->ensure_my_table();
    $placeholder = !empty($this->definition['numeric']) ? '%d' : "'%s'";

    if (count($this->value) == 1) {
      $this->operator = ($this->operator == 'in') ? '= ' : '!= ';
      $in = !empty($this->definition['numeric']) ? '%d' : "'%s'";
    }
    else {
      $replace = array_fill(0, sizeof($this->value), $placeholder);
      $in = ' (' . implode(", ", $replace) . ')';
    }
    $this->query->add_where($this->options['group'], "$this->table_alias.$this->real_field " . $this->operator . $in, $this->value);
  }
}

/**
 * Field handler to provide simple renderer that allows linking to a node.
 */
class workflow_views_handler_field_sid extends views_handler_field {
  function render($values) {
    if (empty($values->{$this->field_alias})) {
      return t('No state');
    }

    static $states;
    if (!isset($states)) {
      $states = workflow_get_states();
    }
    $output = $states[$values->{$this->field_alias}];
    if (empty($output)) {
      $output = t('Unknown state');
    }
    return check_plain($output);
  }
}

/**
 * @}
 */