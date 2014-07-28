package ch.unifr.goal.complement;

/* Binds an action to clicking on the menu item that is defined in plugin.xml */

/* Daniel Weibel, 25.07.2014 */

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
    return "The ground-breaking Büchi complementation algorithm using a subset-tuple construction.";
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
