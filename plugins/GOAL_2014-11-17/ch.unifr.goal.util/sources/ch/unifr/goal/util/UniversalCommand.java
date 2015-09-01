/*============================================================================*
 * Command "universal" for the command line interface of GOAL. An automaton is
 * universal if it accepts ∑*, that is, all possible words over the alphabet ∑.
 * The test is done by complementing the input automaton and testing the
 * complement for emptiness.
 *
 * Author: Daniel Weibel <daniel.weibel@unifr.ch>, 20 Dec. 2014
 *
 * Copyright (c) 2014-2015 Daniel Weibel <daniel.weibel@unifr.ch> 
 *============================================================================*/

package ch.unifr.goal.util;

import java.util.List;
import org.svvrl.goal.cmd.CmdUtil;
import org.svvrl.goal.cmd.Context;
import org.svvrl.goal.cmd.Expression;
import org.svvrl.goal.cmd.CommandExpression;
import org.svvrl.goal.cmd.EvaluationException;
import org.svvrl.goal.cmd.ComplementExpression;
import org.svvrl.goal.core.aut.fsa.FSA;
import org.svvrl.goal.core.aut.fsa.Emptiness;
import org.svvrl.goal.core.aut.fsa.Emptiness.Result;

public class UniversalCommand extends CommandExpression {

  // "complement" -> ComplementExpression, "unviversal" -> UniversalCommand
  ComplementExpression complCmd;

  /* Constructor: initialise "universal" command
   * List<Expression> args: command line arguments
   * goal universal -m rank -r a.gff --> args: ["-m", "rank", "-r", "a.gff"] */
  public UniversalCommand(List<Expression> args) throws EvaluationException {
    super(args);
    for (Expression arg : args) {
      String s = arg.toString();
      // Make sure there are no args understood by "complement" other than "-m".
      // Could lead to wrong result, if e.g. "-s 1" is present.
      if (s.equals("-o") || s.equals("-s") || s.equals("-t"))
        throw new EvaluationException();
    }
    // Argument list is understood as-is by "complement" command
    complCmd = new ComplementExpression(args);
  }

  /* Execute "universal" command
   * Context ctx: variable names and values of script where command is run. */
  @Override
  public Object eval(Context ctx) throws EvaluationException {
    FSA complement = (FSA) complCmd.eval(ctx);
    Emptiness.Result emptResult = Emptiness.isEmpty(complement);
    if (emptResult.isEmpty()) return "true\n";
    else                      return "false\n";
  }

}