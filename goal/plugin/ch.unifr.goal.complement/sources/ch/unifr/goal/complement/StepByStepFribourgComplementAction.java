/*============================================================================*
 * Author: Daniel Weibel <daniel.weibel@unifr.ch>
 * Last modified: 5 Oct. 2014
 *============================================================================*/
package ch.unifr.goal.complement;

import org.svvrl.goal.core.FinishedException;
import org.svvrl.goal.core.aut.fsa.FSA;
import org.svvrl.goal.core.aut.OmegaUtil;
import org.svvrl.goal.gui.Window;
import org.svvrl.goal.gui.ProgressDialog;
import org.svvrl.goal.gui.Tab;
import org.svvrl.goal.gui.ControllableTab;
import org.svvrl.goal.gui.action.EditableAction;
import org.svvrl.goal.gui.pref.OptionsDialog;

/*----------------------------------------------------------------------------*
 * Interface of the Fribourg construction with the GUI. This class defines
 * the action when clicking on the Fribourg Construction menu item under
 * Automaton > Complement (Step-by-Step) in the GUI.
 *----------------------------------------------------------------------------*/
/* javax.swing.AbstractAction > WindowAction > EditableAction */
public class StepByStepFribourgComplementAction extends EditableAction<FSA,Void> {

  private FribourgOptions options;

  /* Constructor */
  public StepByStepFribourgComplementAction(Window win) {
    super(win, "Fribourg Construction"); // Text of menu item and title of status window
  }

  @Override // Method of WindowAction
  public void preProcess(ProgressDialog dialog) throws Exception {
    super.preProcess(dialog);  // Calls isApplicable()
    OptionsDialog<FribourgOptions> optionsDialog = new OptionsDialog<FribourgOptions>(getWindow(), new FribourgComplementOptionsPanel());
    optionsDialog.setTitle("Options for the Fribourg Construction");
    optionsDialog.setVisible(true);
    options = optionsDialog.getOptions();
    // When the user clicks "Cancel" in the options dialog
    if (options == null) throw new FinishedException();
  }

  @Override // Abstract method of WindowAction
  public Void execute(ProgressDialog dialog) {
    final FribourgConstruction constr = new FribourgConstruction(getInput(), options);
    ControllableTab tab = new ControllableTab(constr);
    tab.setEditable(getInput());
    getWindow().addTab(tab);
    Thread t = new Thread() {
      public void run() {
        constr.doPause(); // Without that, the algorithm does not pause at steps and stages
        constr.complement();
      }
    };
    t.start();
    return null;
  }

  // @Override
  // public void postProcess(ProgressDialog dialog) {
  // }

  // Called by preProcess of EditableAction
  @Override // Method of WindowAction
  public boolean isApplicable(Tab tab) {
    if (OmegaUtil.isNBW(getInput())) return true;
    else return false;
  }

  @Override // Abstract method of WindowAction
  public String getToolTip() {
    return "Step-by-step version of the Fribourg construction";
  }

}