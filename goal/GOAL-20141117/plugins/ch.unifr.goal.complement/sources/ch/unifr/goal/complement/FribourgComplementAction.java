/*============================================================================*
 * "Fribourg Construction" menu item in the GUI (under "Automaton > Complement")
 * 
 * Copyright (c) 2014-2015 Daniel Weibel <daniel.weibel@unifr.ch>
 *============================================================================*/

package ch.unifr.goal.complement;

import org.svvrl.goal.core.Properties;
import org.svvrl.goal.core.aut.fsa.FSA;
import org.svvrl.goal.gui.Window;
import org.svvrl.goal.gui.action.ExecutionException;
import org.svvrl.goal.gui.action.AbstractComplementAction;
import org.svvrl.goal.gui.pref.OptionsDialog;

// AbstractAction > WindowAction > EditableAction > AutomatonOperationAction > AbstractComplementAction
public class FribourgComplementAction extends AbstractComplementAction<FribourgConstruction> {

  /* Constructor */
  public FribourgComplementAction(Window win) {
    super(win, "Fribourg Construction"); // Text of menu item and title of status window
  }

  /* Create a FribourgConstruction with FribourgOptions */
  @Override // Abstract method of AbstractComplementAction
  protected FribourgConstruction getConstruction(FSA in, Properties options) {
    FribourgOptions fribourgOptions = (FribourgOptions) options;
    return new FribourgConstruction(in, fribourgOptions);
    // FribourgOptions options;
    // if (o instanceof FribourgOptions) options = (FribourgOptions) o;
    // else options = new FribourgOptions(o);
    // return new FribourgConstruction(in, options);
  }

  /* Create the dialog that appears between clicking the menu item and the start
   * of the complementation. */
  @Override // Abstract method of AbstractComplementAction
  protected OptionsDialog<FribourgOptions> getOptionsDialog(Window win) {
    OptionsDialog<FribourgOptions> dialog = new OptionsDialog<FribourgOptions>(win, new FribourgComplementOptionsPanel());
    dialog.setTitle("Options for the Fribourg Construction");
    return dialog;
  }

  @Override // Abstract method of AbstractComplementAction
  protected Class<FribourgConstruction> getConstructionClass() {
    return FribourgConstruction.class;
  }

  @Override // Abstract method of WindowAction
  public String getToolTip() {
    return "Slice-based complementation construction developed at the University of Fribourg, Switzerland.";
  }
}
