/*============================================================================*
 * Author:        Daniel Weibel <daniel.weibel@unifr.ch>
 * Last modified: 19 Dec. 2014
 *============================================================================*/
package ch.unifr.goal.util;

import java.util.List;
import java.util.LinkedList;
import java.util.Collection;
import java.util.ListIterator;
import org.svvrl.goal.core.aut.State;
import org.svvrl.goal.core.aut.Automaton;
import org.svvrl.goal.core.aut.fsa.FSA;
import org.svvrl.goal.cmd.Context;
import org.svvrl.goal.cmd.Expression;
import org.svvrl.goal.cmd.CommandExpression;
import org.svvrl.goal.cmd.EvaluationException;
import org.svvrl.goal.cmd.CmdUtil;

import org.svvrl.goal.cmd.ComplementExpression;
import org.svvrl.goal.core.aut.fsa.Emptiness;

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

  // Argument specifying automaton
  Expression autArg;

  // Information deduced from args specifying complementation construction
  String           complConstr;
  List<Expression> complOpts        = new LinkedList<Expression>();
  boolean          useDefComplOpts = false;

  String expandedOpts = "Expanded options:\n";
  List<Object> exprOpts = new LinkedList<Object>();

  ComplementExpression complCmd;

  /* Constructor: parse command line arguments
   * args: command line arguments
   * goal universal -m rank -s a.gff --> args: ["-m", "rank", "-s", "a.gff"] */
  public UniversalCommand(List<Expression> args) throws EvaluationException{
    super(args);

    complCmd = new ComplementExpression(args);
    // int argc = args.size();
    // // Last arg specifies automaton
    // autArg = args.get(argc - 1);
    // // Parse other args
    // ListIterator<Expression> iter = args.subList(0, argc - 1).listIterator();
    // while (iter.hasNext()) {
    //   Expression e = iter.next();
    //   String     s = e.toString();
    //   if      (s.equals("-m"))  complConstr      = iter.next().toString();
    //   else if (s.equals("-s"))  useDefComplOpts  = true;
    //   else                      complOpts.add(e);
    // }

    // ListIterator<Expression> iter = args.listIterator();
    // while (iter.hasNext()) {
    //   Expression e = iter.next();
    //   expandedOpts += e.toString() + " | ";
    // }
  }

  /* Test if the specified automaton is complete. Returns the string "true" or
   * "false" which is sent to stdout.
   * Context ctx: contains all variable names and their values of the script
   * where the universal command is executed. */
  @Override
  public Object eval(Context ctx) throws EvaluationException {
    FSA compl = (FSA) complCmd.eval(ctx);
    /* Evaluate the automaton argument. It may be:
     *   - A filename on the command line
     *   - A variable containing a filename in a script
     *   - A variable containing an automaton in a script
     *   - A filename in a script
     * castOrLoadAutomaton handles all these cases (if the arg is a filename
     * tries all the known codecs) and decodes the argument as an Automaton. */
    // Evaluate expression. If autArg is a variable, expand it. Result is either
    // a filename or a string or script-internal representatio of an automaton.
    // Object o = autArg.eval(ctx);
    // Create an Automaton object from the automaton specification o
    // Automaton aut = CmdUtil.castOrLoadAutomaton(o);

    

    // if (isComplete(aut)) return "true";
    // else                 return "false";
    return CmdUtil.encodeEditableAsString(compl);
  }


  /* Return true if the automaton is complete and false otherwise */
  private boolean isComplete(Automaton a) {
    for (State s : a.getStates())
      if (!(a.getSymbolsFromState(s).size() == a.getAlphabet().length)) return false;
    return true;
  }
}