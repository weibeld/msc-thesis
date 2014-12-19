/*============================================================================*
 * Author:        Daniel Weibel <daniel.weibel@unifr.ch>
 * Last modified: 19 Dec. 2014
 *============================================================================*/
package ch.unifr.goal.util;

import java.util.List;
import org.svvrl.goal.core.aut.State;
import org.svvrl.goal.core.aut.Automaton;
import org.svvrl.goal.cmd.Context;
import org.svvrl.goal.cmd.Expression;
import org.svvrl.goal.cmd.CommandExpression;
import org.svvrl.goal.cmd.EvaluationException;
import org.svvrl.goal.cmd.CmdUtil;


/*----------------------------------------------------------------------------*
 * A command line command for testing if an automaton is complete. An automaton
 * is complete if every state has an outgoing transition for every symbol of
 * the alphabet.
 *----------------------------------------------------------------------------*/
public class CompleteCommand extends CommandExpression {

  /* Argument specifying the automaton to test */
  Expression autArg;

  /* Constructor: parse command line arguments
   * args: command line arguments
   * E.g. goal complete -a -b aut.gff --> args: ["-a", "-b", "aut.gff"] */
  public CompleteCommand(List<Expression> args) throws EvaluationException{
    super(args);
    int argc = args.size();
    /* All but the last argument are taken as options */
    List<Expression> optArgs = args.subList(0, argc - 1);
    /* The last argument specifies the automaton to test */
    autArg = args.get(argc - 1);

    /* Parse options. Currently, the command has no options. */
    for (Expression item : optArgs) {
      String opt = item.toString();
      // Test if opt is an allowed option
      throw new EvaluationException();
    }
  }

  /* Test if the specified automaton is complete. Returns the string "true" or
   * "false" which is sent to stdout. */
  @Override
  public Object eval(Context ctx) throws EvaluationException {
    /* Evaluate the automaton argument. It may be:
     *   - A filename on the command line
     *   - A variable containing a filename in a script
     *   - A variable containing an automaton in a script
     *   - A filename in a script
     * castOrLoadAutomaton handles all these cases (if the arg is a filename
     * tries all the known codecs) and decodes the argument as an Automaton. */
    Automaton aut = CmdUtil.castOrLoadAutomaton(autArg.eval(ctx));
    if (isComplete(aut)) return "true";
    else                 return "false";
  }


  /* Return true if the automaton is complete and false otherwise */
  private boolean isComplete(Automaton a) {
    for (State s : a.getStates())
      if (!(a.getSymbolsFromState(s).size() == a.getAlphabet().length)) return false;
    return true;
  }
}