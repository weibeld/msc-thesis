package ch.unifr.goal.hello;

import java.util.List;

import org.svvrl.goal.cmd.AbstractCommandExtension;
import org.svvrl.goal.cmd.CommandExpression;
import org.svvrl.goal.cmd.Context;
import org.svvrl.goal.cmd.EvaluationException;
import org.svvrl.goal.cmd.Expression;


/* There's the interface CommandExtension, defining methods setName, getName,
 * setFullName, getFullName, parse, and getHelp. The class
 * AbstractCommandExtension implements the first four methods. This means that
 * we have to implement parse and getHelp in our class. */
public class HelloCommand extends AbstractCommandExtension {

  /* There's the interface Expression, defining methods boolValue, doubleValue,
   * floatValue, intValue, stringValue, and eval. The class AbstractExpression
   * implements the first five of these methods. CommandExpression is a
   * subclass of AbstractExpression, implements mehods getArguments,
   * parseBoolean, and toString, and has abstract method getName. Thus, in our
   * class we need to implement eval (from Expression) and getName. */
  private class HelloExpression extends CommandExpression {

    public HelloExpression(List<Expression> args) {
      super(args);
    }
    
    @Override
    public Object eval(Context context) throws EvaluationException {
      return "Hello";
    }

    @Override
    public String getName() {
      return "hello";
    }

  }

  @Override
  public String getHelp() {
    return "Usage: goal hello <word>\nOutputs \"Hello <word>\"";
  }

  @Override
  public CommandExpression parse(List<Expression> args) throws IllegalArgumentException {
    if (args.size() < 1) throw new IllegalArgumentException("Command must have exactly one parameter.");
    else return new HelloExpression(args);
  }
}