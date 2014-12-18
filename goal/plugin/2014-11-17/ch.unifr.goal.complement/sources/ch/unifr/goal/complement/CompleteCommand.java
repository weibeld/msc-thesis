package ch.unifr.goal.complement;

import java.util.List;
import org.svvrl.goal.cmd.Lval;
import org.svvrl.goal.cmd.Context;
import org.svvrl.goal.cmd.Expression;
import org.svvrl.goal.cmd.CommandExpression;
import org.svvrl.goal.cmd.EvaluationException;
import org.svvrl.goal.core.aut.Automaton;
import org.svvrl.goal.core.aut.State;
import org.svvrl.goal.core.io.FileHandler;
import org.svvrl.goal.core.util.Pair;
import org.svvrl.goal.core.Editable;
import org.svvrl.goal.core.io.Codec;
import java.io.IOException;
import org.svvrl.goal.core.UnsupportedException;

public class CompleteCommand extends CommandExpression {

  String options = "Options:\n";
  String automaton = "Automaton:\n";
  Automaton aut;

  /* Constructor
   * args: command line arguments
   * E.g. goal complete -a -b aut.gff --> args: ["-a", "-b", "aut.gff"] */
  public CompleteCommand(List<Expression> args) throws EvaluationException{
    super(args);
    // Number of arguments
    int argc = args.size();
    // All but the last argument are taken as options
    List<Expression> optsExpr = args.subList(0, argc - 1);
    // The last argument specifies the automaton
    Expression autExpr = args.get(argc - 1);

    // Currently, the command has no options
    for (Expression item : optsExpr) {
      String opt = item.toString();
      throw new EvaluationException("Unknown option: " + opt);
    }

    // Currently, we assume that the automaton specification is a file (so, the
    // command will not work in scripts).
    String filename = autExpr.toString();
    FileHandler fh = new FileHandler();
    try {
      Pair<Editable, Codec> pair = fh.open(filename);
      aut = (Automaton) pair.getLeft();
    }
    catch (Exception e) {
      throw new EvaluationException("ERROR: " + filename + " is not a valid automaton.");
    }
  }

  @Override
  public Object eval(Context ctx) throws EvaluationException {
    if (isComplete(aut)) return "true\n";
    else                 return "false\n";
  }

  /* Test if an automaton is complete */
  private boolean isComplete(Automaton a) {
    for (State s : a.getStates())
      if (!(a.getSymbolsFromState(s).size() == a.getAlphabet().length)) return false;
    return true;
  }
}