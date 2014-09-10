package ch.unifr.goal.complement;

import org.svvrl.goal.core.aut.fsa.FSA;
import org.svvrl.goal.gui.Window;
import org.svvrl.goal.gui.action.EditableAction;
import org.svvrl.goal.gui.ProgressDialog;
import org.svvrl.goal.gui.Tab;
import org.svvrl.goal.gui.ControllableTab;
import org.svvrl.goal.gui.pref.OptionsDialog;
import org.svvrl.goal.core.FinishedException;

/* javax.swing.AbstractAction > WindowAction > EditableAction */
public class StepByStepFribourgComplementAction extends EditableAction<FSA,Void> {

  private FribourgOptions options;

  /* Constructor */
  public StepByStepFribourgComplementAction(Window win) {
    super(win, "Fribourg Construction"); // Text of menu item and title of status window
  }

  @Override // Method of EditableAction
  public void preProcess(ProgressDialog dialog) throws Exception {
    super.preProcess(dialog);
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
    //tab.setEditable(constr.getIntermediateResult());
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

  @Override // Method of EditableAction
  public boolean isApplicable(Tab tab) {
    return true;
  }

  /* Text when hovering over the menu item */
  @Override // Abstract method of WindowAction
  public String getToolTip() {
    return "Step-by-step version of the Fribourg construction";
  }

}