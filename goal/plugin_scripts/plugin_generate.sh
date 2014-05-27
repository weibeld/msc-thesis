#!/bin/bash
# dw-26.05.2014

# Generates an example JPF plugin for GOAL in the directory given as argument
# (will be created). The example plugin can be used as a starting point
# for creating an own custom plugin. It is ready to build with plugin_build.sh.
# It adds a Büchi complementation construction to GOAL and makes it accessible
# from the GUI as well as fromthe command line.
# Assumes that the name of the plugin directory is a Java package name. This
# is a convention and necessary for plugin_build.sh to work.

# Exit if any command returns a non-zero status (i.e. error)
set -e

if [ $# -lt 1 ]; then
  echo "Usage: $(basename $0) <plugin dir (will be created)>"
  exit 0
fi

PLUGIN_DIR=$1
mkdir $PLUGIN_DIR
cd $PLUGIN_DIR


# Create the sources directory tree. This is possible thanks to the convention
# that the plugin directory name is the name of the Java package of the newly
# created plugin.
PACKAGE=$(basename $PLUGIN_DIR)
IFS='.' read -a ARRAY <<< $PACKAGE
SOURCE_PATH='sources'
for ELT in ${ARRAY[@]}; do SOURCE_PATH=$SOURCE_PATH/$ELT; done
mkdir -p $SOURCE_PATH


# Will contain the classes directory tree after compilation.
mkdir classes


# The example plugin manifest, plugin.xml in the root directory of the plugin.
cat >plugin.xml <<EOF
<?xml version="1.0" ?>
<!DOCTYPE plugin PUBLIC "-//JPF//Java Plug-in Manifest 1.0" "http://jpf.sourceforge.net/plugin_1_0.dtd">

<!--
This manifest and the code of this example plugin add an additional Büchi
complementation construction to GOAL: as a menu item in the GUI, and as an
option to the 'complement' command on the command line.
-->

<plugin id="$PACKAGE" version="0.0.1">
  
  <!-- The plugins whose code is accessed by this plugin -->
  <requires>
    <import plugin-id="org.svvrl.goal.core" />
    <import plugin-id="org.svvrl.goal.gui" />
    <import plugin-id="org.svvrl.goal.cmd" />
  </requires>


  <!-- The location of the code of this plugin -->
  <runtime>
    <library id="$PACKAGE" path="classes/" type="code">
      <!-- Uncomment if other plugins need to access the code of this plugin -->
      <!-- <export prefix="*" /> -->
    </library>
  </runtime>


  <!-- Extending the GUI menu with an additional menu item: an item under the
       sub-menu Automaton > Complement -->
  <extension plugin-id="org.svvrl.goal.gui" point-id="Menu" id="FribourgComplementAction">
    <parameter id="class" value="$PACKAGE.FribourgComplementAction" />
    <parameter id="container" value="org.svvrl.goal.gui.menu.ComplementMenu" />
    <parameter id="location" value="first" />
  </extension>


  <!-- Extending the command line command 'complement' with an additional
       algorithm, i.e. 'goal complement -m fribourg foo.gff' -->
  <extension plugin-id="org.svvrl.goal.cmd" point-id="ComplementCommand" id="FribourgComplementExtension">
    <parameter id="class" value="$PACKAGE.FribourgComplementExtension" />
    <!-- The name of the command on the command line -->
    <parameter id="name" value="fribourg" />
  </extension>

</plugin>
EOF


# The Java source files for the example plugin
cat >$SOURCE_PATH/FribourgComplementAction.java <<EOF
package $PACKAGE;

/* Binds an action to the clicking of the menu item defined in plugin.xml */

import org.svvrl.goal.core.FinishedException;
import org.svvrl.goal.core.aut.Automaton;
import org.svvrl.goal.core.aut.OmegaUtil;
import org.svvrl.goal.core.aut.fsa.FSA;
import org.svvrl.goal.gui.ProgressDialog;
import org.svvrl.goal.gui.Window;
import org.svvrl.goal.gui.action.AutomatonOperationAction;
import org.svvrl.goal.gui.action.ExecutionException;

/* Class Hierarchy: Object > AbstractAction > WindowAction > EditableAction >
 * AutomatonOperationAction
 * AutomatonOperationAction: abstraction for operations on automata that produce
 * new automata. */
public class FribourgComplementAction extends AutomatonOperationAction<FSA, FSA> {

  /* Constructor
   * The string argument to super() is the label of the menu item. */
  public FribourgComplementAction(Window win) {
    super(win, "Fribourg Construction");
  }


  /* Abstract method from WindowAction */
  @Override
  public String getToolTip() {
    return "The ground-breaking Büchi complementation construction from the University of Fribourg.";
  }


  /* Abstract method from AutomatonOperationAction
   * Returns true if a specified automaton is supported by this action. */
  @Override
  public boolean isApplicable(Automaton aut) {
    // Applicable only for nondeterministic Büchi automata
    return OmegaUtil.isNBW(aut);
  }


  /* Abstract method from WindowAction
   * Binds a concrete action to this menu item. */
  @Override
  public FSA execute(ProgressDialog dialog) throws ExecutionException, FinishedException {
    // getInput: method of EditableAction. Returns the input of this action,
    // i.e. the FSA to be complemented.
    // complement: returns an FSA (the complement of the input automaton of
    // FribourgConstruction).
    return new FribourgConstruction(getInput().clone()).complement();
  }

}
EOF

cat >$SOURCE_PATH/FribourgConstruction.java <<EOF
package $PACKAGE;

/* Contains the actual complementation construction */

import org.svvrl.goal.core.Editable;
import org.svvrl.goal.core.Message;
import org.svvrl.goal.core.aut.BuchiAcc;
import org.svvrl.goal.core.aut.OmegaUtil;
import org.svvrl.goal.core.aut.State;
import org.svvrl.goal.core.aut.StateSet;
import org.svvrl.goal.core.aut.fsa.FSA;
import org.svvrl.goal.core.comp.ComplementConstruction;


/* Class hierarchy: Object > AbstractAlgorithm > AbstractControllableAlgorithm >
 * AbstractEditableAlgorithm > ComplementConstruction */
public class FribourgConstruction extends ComplementConstruction<FSA, FSA> {

  /* Holding the complement automaton if complement() has already been
   * executed before. */
  private FSA complement = null;


  /* Constructor
   * aut is the input automaton to this construction. */
  public FribourgConstruction(FSA aut) {
    super(aut);
    if (!OmegaUtil.isNBW(aut))
      throw new IllegalArgumentException(Message.onlyForFSA(BuchiAcc.class));
  }


  /* Method from interface EditableAlgorithm */
  @Override
  public Editable getIntermediateResult() {
    return complement;
  }


  /* Abstract method from ComplementConstruction
   * Performs the complementation and returns the complement. */
  @Override
  public FSA complement() {
    // If complementation has already been done before.
    if (complement != null) return complement;
    // Method from AbstractControllableAlgorithm
    fireReferenceChangedEvent();
    // getInput: ComplementConstruction. Returns automaton to be complemented.
    complement = dual(getInput().clone());
    return complement;
  }


  /* Dummy for the implementation of the real complementation construction.
   * Return the dual automaton (i.e. accepting and non-accepting states
   * switched) of the automaton received as argument. */
  private FSA dual(FSA input) {
    StateSet states = new StateSet(input.getStates());
    BuchiAcc acceptingSet = (BuchiAcc) input.getAcc();
    states.removeAll(acceptingSet.get());
    acceptingSet.clear();
    for (State s : states) acceptingSet.add(s);
    return input;
  }

}
EOF

cat >$SOURCE_PATH/FribourgComplementExtension.java <<EOF
package $PACKAGE;

/* Binds an action to the command line command defined in plugin.xml */

import java.util.List;
import org.svvrl.goal.cmd.AbstractCommandExtension;
import org.svvrl.goal.cmd.CommandExpression;
import org.svvrl.goal.cmd.ComplementCommand;
import org.svvrl.goal.cmd.Expression;
import org.svvrl.goal.core.aut.fsa.FSA;
import org.svvrl.goal.core.comp.ComplementConstruction;


/* Class hierarchy: Object > AbstractCommandExtension */
public class FribourgComplementExtension extends AbstractCommandExtension {

  /* Class hierarchy: Object > AbstractExpression > CommandExpression >
   * ComplementCommand */
  class FribourgComplementCommand extends ComplementCommand {
    /* Constructor
     * args: the command line arguments */
    public FribourgComplementCommand(List<Expression> args) {
      super(args);
    }
    /* Abstract method from ComplementCommand
     * Returns the FribourgConstruction which is the same used by the GUI
     * class FribourgComplementAction. */
    @Override
    public ComplementConstruction<?, ?> getComplementConstruction(FSA aut) {
      return new FribourgConstruction(aut);
    }
  }


  /* Method from the interface CommandExtension
   * No help, because this is only an option to a command and not a command. */
  @Override
  public String getHelp() { return ""; }


  /* Method from the interface CommandExtension
   * Parses arguments and constructs a command expression. */
  @Override
  public CommandExpression parse(List<Expression> args) throws IllegalArgumentException {
    return new FribourgComplementCommand(args);
  }

}
EOF

echo "Created example plugin at $(pwd)"
