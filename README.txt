********************************************************************
                     D R U P A L    M O D U L E
********************************************************************
Name: Workflow Module
Author: John VanDyk <jvandyk at iastate dot edu>
Drupal: 4.5
********************************************************************
DESCRIPTION:

The workflow module enables you to create arbitrary workflows in 
Drupal and associate them with node types.

Workflows are made up of workflow states.

Moving from one state to another is called a transition.

Actions are associated with transitions (actions.module must be
installed for this).

********************************************************************
INSTALLATION:

1. Place the entire workflow directory into your Drupal modules/
   directory.

2. Create the necessary database tables using the workflow.mysql
   file and the tool of your choice. For example, you could use
   PhpMyAdmin, to upload the workflow.mysql file or run the following
   from the command line:

     mysql -u user -p drupal < workflow.mysql

   Replace 'user' with the MySQL username that has permission to modify
   the database, and 'drupal' with the database being written to. The
   database must already be created.

3. Enable the workflow module by navigating to:

     administer > modules

4. If you want anyone besides the administrative user to be able
   to configure workflows (usually a bad idea), they must be given
   the "administer workflow" access permission:
   
     administer > users > configure > permissions

   When the module is enabled and the user has the "administer
   workflow" permission, a "workflow" menu should appear in the 
   menu system.

********************************************************************
NOTES:

If you get an error that looks like

"user error: Table 'drupal.workflows' doesn't exist"

when you go to admin/workflows, it means that you have missed step 2,
above.

********************************************************************
GETTING STARTED:

Let's create a new workflow. Click on administer -> workflow and click
on the "add workflow" tab.

We'll start simple. Call our workflow "Draft-Done" and click Add Workflow.

Now lets add some workflow states to our workflow. Click "add state" and
enter "draft" and click the Add State button. Do the same for "done".

So we've got a workflow with two states, "draft" and "done". Now we
have to tell each state which other states it can move to. With only
two states, this is easy. Click on the "transitions" link to the right
of the "draft" state.

The "From" column lists all states that may move to this state. The
"To" column lists all states that we may move to from this state. Since
we're keeping this simple, we will have the simplest possible workflow.
Nodes in a "draft" state may move to a "done" state, and that's it.
To set this up, click the checkbox next to "done" and click the
Save Transitions button.

Just for fun, click on the transitions link to the right of the "done"
state. As expected, it shows that you can get to the "done" state
from the "draft" state.

Now let's tell Drupal which node types should use this workflow. Click
on the "type mapping" tab. Let's assign the Draft-Done workflow
to the story node type and click Save Workflow Mapping.

Now we could add an action (previously configured using the actions
module). If you have not configured your action yet, here's a quick
guide:

- go to administer -> actions
- choose an action and click "Add new action"
- configure the action and click Save. Note that the description field
  here will be used later on

OK, back in the workflow module, click on the actions link to the
right of your workflow. Add the action to the transition.

Now create a new story by going to create content -> story. Note that
there is no sign of workflow here because the story is in its
initial state. Click submit to create the story.

Now click the edit tab. Note that there is a select box for workflow
with the "draft" state chosen.

Changing the state to "done" and clicking Submit will fire the action
you set up earlier.


********************************************************************
TO DO:

Suppress transitions link if only one state is defined.

Notify user that transitions must exist prior to actions being associated.

Action menu is showing state mapping to itself. Bug or feature?