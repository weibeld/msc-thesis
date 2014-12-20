/*============================================================================*
 * Author: Daniel Weibel <daniel.weibel@unifr.ch>, 20 Dec. 2014
 *============================================================================*/
package ch.unifr.goal.test;

/* Basic imports (always required) */
import java.util.List;
import org.svvrl.goal.cmd.Context;
import org.svvrl.goal.cmd.Expression;
import org.svvrl.goal.cmd.CommandExpression;
import org.svvrl.goal.cmd.EvaluationException;

/* Test imports */
import org.svvrl.goal.core.comp.ComplementRepository;


public class MyTestCommand extends CommandExpression {

  public MyTestCommand(List<Expression> args) throws EvaluationException {
    super(args);
  }


  @Override
  public Object eval(Context ctx) throws EvaluationException {
    return ComplementRepository.getNames();
  }

}