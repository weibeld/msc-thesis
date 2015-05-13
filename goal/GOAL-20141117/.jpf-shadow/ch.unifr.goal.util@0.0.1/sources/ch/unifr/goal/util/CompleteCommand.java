/*============================================================================*
 * Author: Daniel Weibel <daniel.weibel@unifr.ch>, 19 Dec. 2014
 *============================================================================*/
package ch.unifr.goal.util;

import java.util.List;
import org.svvrl.goal.core.aut.State;
import org.svvrl.goal.core.aut.fsa.FSA;
import org.svvrl.goal.cmd.Context;
import org.svvrl.goal.cmd.Expression;
import org.svvrl.goal.cmd.CommandExpression;
import org.svvrl.goal.cmd.EvaluationException;
import org.svvrl.goal.cmd.CmdUtil;


/*----------------------------------------------------------------------------*
 * "complete" command line command. An automaton is complete if every state has
 * an outgoing transition for every symbol of the alphabet.
 *----------------------------------------------------------------------------*/
public class CompleteCommand extends CommandExpression {

  // Last argument specifying automaton
  Expression aut;

  /* Constructor: initialise "complete" command
   * List<Expression> args: command line arguments
   * E.g. goal complete -a -b aut.gff --> args: ["-a", "-b", "aut.gff"] */
  public CompleteCommand(List<Expression> args) throws EvaluationException{
    super(args);
    int argc = args.size();
    aut = args.get(argc - 1);  // Last arg specifies automaton
    // Remaining args are options. Currently no options.
    for (Expression arg : args.subList(0, argc - 1)) {
      throw new EvaluationException();
    }
  }

  /* Execute "complete" command
   * Context ctx: variable names and values of script where command is run. */
  @Override
  public Object eval(Context ctx) throws EvaluationException {
    // Evaluate automaton arg. Required because might be a GOAL variable.
    Object o = aut.eval(ctx);
    // o might be filename, XML string, etc., transform it to FSA object
    FSA a = CmdUtil.castOrLoadFSA(o);
    if (isComplete(a)) return "true";
    else               return "false";
  }


  /* Return true if the automaton is complete and false otherwise */
  private boolean isComplete(FSA a) {
    for (State s : a.getStates())
      if (!(a.getSymbolsFromState(s).size() == a.getAlphabet().length)) return false;
    return true;
  }
}