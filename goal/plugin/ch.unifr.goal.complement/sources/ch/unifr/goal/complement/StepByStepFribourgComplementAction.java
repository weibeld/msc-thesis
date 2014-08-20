package ch.unifr.goal.complement;

import org.svvrl.goal.core.aut.fsa.FSA;
import org.svvrl.goal.gui.Window;
import org.svvrl.goal.gui.action.EditableAction;
import org.svvrl.goal.gui.ProgressDialog;
import org.svvrl.goal.gui.Tab;
import org.svvrl.goal.gui.ControllableTab;
import org.svvrl.goal.gui.pref.OptionsDialog;

/* javax.swing.AbstractAction > WindowAction > EditableAction */
public class StepByStepFribourgComplementAction extends EditableAction<FSA,Void> {

  /* Constructor */
  public StepByStepFribourgComplementAction(Window win) {
    super(win, "Fribourg Construction"); // Text of menu item and title of status window
  }

  @Override // Abstract method of WindowAction
  public Void execute(ProgressDialog dialog) {
    final FribourgConstruction constr = new FribourgConstruction(getInput(), new FribourgOptions());
    ControllableTab tab = new ControllableTab(constr);
    getWindow().addTab(tab);
    OptionsDialog<FribourgOptions> dia = new OptionsDialog<FribourgOptions>(getWindow(), new FribourgComplementOptionsPanel());
    dia.setTitle("Options for the Fribourg Construction");
    getWindow().add(dia);
    Thread t = new Thread() {
      public void run() {
        constr.doPause();
        constr.complement();
      }
    };
    t.start();
    return null;
  }

  @Override // Method of EditableAction
  public boolean isApplicable(Tab tab) {
    return true;
  }

  @Override // Method of EditableAction
  public void preProcess(ProgressDialog dialog) {
  }

  @Override // Method of WindowAction
  public int getMnemonic() {
    return 0;
  }

  /* Text when hovering over the menu item */
  @Override // Abstract method of WindowAction
  public String getToolTip() {
    return "Step-by-step version of the Fribourg construction";
  }

}