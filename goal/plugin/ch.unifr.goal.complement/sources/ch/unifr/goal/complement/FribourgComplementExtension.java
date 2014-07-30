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

    private FribourgOptions options;

    /* Constructor
     * args: the command line arguments */
    public FribourgComplementCommand(List<Expression> args) throws IllegalArgumentException {
      super(args);
      // Create FribourgOptions with default values
      // TODO: can't call set<Option>() directly because of the restriction that
      // ignoreRightColor2 is only accepted if makeComplete is true.
      options = new FribourgOptions();
      for (Expression arg : args) {
        String opt = arg.toString();
        if (opt.equals("-c")) options.setMakeComplete(true);
        else if (opt.equals("-r2")) options.setIgnoreRightColor2(true);
        else throw new IllegalArgumentException("Unknown option: " + opt);
      }
    }
    /* Abstract method from ComplementCommand
     * Returns the FribourgConstruction which is the same used by the GUI
     * class FribourgComplementAction. */
    @Override
    public ComplementConstruction<?, ?> getComplementConstruction(FSA in) {
      return new FribourgConstruction(in, options);
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
