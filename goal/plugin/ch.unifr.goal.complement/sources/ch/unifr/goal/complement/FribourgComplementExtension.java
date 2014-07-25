package ch.unifr.goal.complement;

/* Binds an action to the command line command (option to complement) that is
 * defined in plugin.xml */
/* Daniel Weibel, 25.07.2014 */

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
    public ComplementConstruction<?, ?> getComplementConstruction(FSA in) {
      return new FribourgConstruction(in);
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
