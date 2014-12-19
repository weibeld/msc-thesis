/*============================================================================*
 * Author:        Daniel Weibel <daniel.weibel@unifr.ch>
 * Last modified: 19 Dec. 2014
 *============================================================================*/
package ch.unifr.goal.util;

import java.util.List;
import java.util.LinkedList;
import java.util.Collection;
import org.svvrl.goal.core.aut.State;
import org.svvrl.goal.core.aut.Automaton;
import org.svvrl.goal.cmd.Context;
import org.svvrl.goal.cmd.Expression;
import org.svvrl.goal.cmd.CommandExpression;
import org.svvrl.goal.cmd.EvaluationException;
import org.svvrl.goal.cmd.CmdUtil;

import org.svvrl.goal.core.aut.fsa.Emptiness;
import org.svvrl.goal.core.aut.fsa.Complementation; // static method complement(FSA aut): Complements a FSA by the default complementation construction


/*----------------------------------------------------------------------------*
 * A command line command for testing if an automaton is universal. An automaton
 * is universal if it accepts ∑*, that is, all possible words over alphabet ∑.
 *----------------------------------------------------------------------------*/
 
/* The universality test is done by complementation and testing for emptiness.
 * An automaton A is universal if and only if its complement A' is empty.
 * Syntax:
 *   goal universal aut.gff                         complementation with default complementation construction
 *   goal universal -c piterman aut.gff:            complemntation with piterman construction with default options
 *   goal universal -c piterman -eq -ro -- aut.gff: complementation with piterman with custom options
 *   goal universal -c piterman -- aut.gff:         complementation with piterman without options
 */
public class UniversalCommand extends CommandExpression {

  /* Argument specifying the automaton to test */
  Expression autArg;

  String expandedOpts = "Expanded options:\n";
  List<Object> exprOpts = new LinkedList<Object>();

  /* Constructor: parse command line arguments
   * args: command line arguments
   * E.g. goal universal -a -b aut.gff --> args: ["-a", "-b", "aut.gff"] */
  public UniversalCommand(List<Expression> args) throws EvaluationException{
    super(args);
    int argc = args.size();
    /* All but the last argument are taken as options */
    List<Expression> optArgs = args.subList(0, argc - 1);
    /* The last argument specifies the automaton to test */
    autArg = args.get(argc - 1);

    for (Expression a : args) {
      expandedOpts += a.toString() + " ";
    }
    

    // /* Parse options. Currently, the command has no options. */
    // for (int i = 0; i < optArgs.size(); i++) {
    //   String arg = optArgs.get(i).toString();
    //   if (arg.equals("--option")) {
    //     //exprOpts.addAll(CmdUtil.castAsCollection(optArgs.get(++i)));
    //   }
    //   else {
    //     exprOpts.add(optArgs.get(i));
    //   }
    //   // Test if opt is an allowed option
    //   //throw new EvaluationException();
    // }

    // for (Object item : exprOpts) {
    //   Expression e = (Expression) item;
    //   expandedOpts += e.toString() + " ";
    // }
  }

  // private String expandVar(Expression var) {
  //   String res = "";
  //   Collection coll = CmdUtil.castAsCollection(var);
  //   for (Object c : coll) {
  //     res += c.toString();
  //   }
  //   return res;
  // }

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
    // Automaton aut = CmdUtil.castOrLoadAutomaton(autArg.eval(ctx));
    // if (isComplete(aut)) return "true";
    // else                 return "false";
    return expandedOpts;
  }


  /* Return true if the automaton is complete and false otherwise */
  private boolean isComplete(Automaton a) {
    for (State s : a.getStates())
      if (!(a.getSymbolsFromState(s).size() == a.getAlphabet().length)) return false;
    return true;
  }
}