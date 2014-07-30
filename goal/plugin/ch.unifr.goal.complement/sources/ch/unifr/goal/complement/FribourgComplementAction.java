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
import org.svvrl.goal.gui.action.AbstractComplementAction;
import org.svvrl.goal.core.Properties;
import org.svvrl.goal.gui.pref.OptionsDialog;


/* Object > AbstractAction > WindowAction > EditableAction > AutomatonOperationAction >
 * AbstractComplementAction */
public class FribourgComplementAction extends AbstractComplementAction<FribourgConstruction> {

  public FribourgComplementAction(Window win) {
    super(win, "Fribourg Construction");
  }

  @Override // Abstract method of AbstractComplementAction
  protected FribourgConstruction getConstruction(FSA in, Properties o) {
    FribourgOptions options;
    if (o instanceof FribourgOptions) options = (FribourgOptions) o;
    else options = new FribourgOptions(o);
    return new FribourgConstruction(in, options);
  }

  @Override // Abstract method of AbstractComplementAction
  protected OptionsDialog<FribourgOptions> getOptionsDialog(Window win) {
    return new OptionsDialog<FribourgOptions>(win, new FribourgComplementOptionsPanel());
  }

  @Override // Abstract method of AbstractComplementAction
  protected Class<FribourgConstruction> getConstructionClass() {
    return FribourgConstruction.class;
  }

  @Override // Abstract method of WindowAction
  public String getToolTip() {
    return "The ground-breaking Büchi complementation algorithm using a subset-tuple construction.";
  }

}
