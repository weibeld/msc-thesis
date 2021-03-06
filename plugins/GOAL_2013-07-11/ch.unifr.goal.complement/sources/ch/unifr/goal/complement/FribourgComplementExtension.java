/*============================================================================*
 * Author: Daniel Weibel <daniel.weibel@unifr.ch>
 * Last modified: 5 Oct. 2014
 *============================================================================*/
package ch.unifr.goal.complement;

import java.util.List;
import org.svvrl.goal.cmd.AbstractCommandExtension;
import org.svvrl.goal.cmd.CommandExpression;
import org.svvrl.goal.cmd.ComplementCommand;
import org.svvrl.goal.cmd.Expression;
import org.svvrl.goal.core.Properties;
import org.svvrl.goal.core.aut.fsa.FSA;
import org.svvrl.goal.core.comp.ComplementConstruction;


/*----------------------------------------------------------------------------*
 * Interface of the Fribourg construction with the command line. This class
 * creates the 'fribourg' in 'goal complement -m fribourg'.
 *----------------------------------------------------------------------------*/
// Object > AbstractCommandExtension
public class FribourgComplementExtension extends AbstractCommandExtension {

  // The command line codes for all the options for 'fribourg'
  private final String cCode     = "-c";       // Make complete
  private final String r2ifcCode = "-r2ifc";   // Right colour 2 if complete
  private final String mCode     = "-m";       // Merge components
  private final String m2Code    = "-m2";      // Reduce colour 2 components
  private final String bCode     = "-b";       // Use the bracket notation
  private final String maccCode  = "-macc";    // Maximise accepting set
  private final String rCode     = "-r";       // Remove unreachable and dead from OUTPUT
  private final String rrCode    = "-rr";      // Remove unreachable and dead from INPUT

  /* Inner class creating a FribourgConstruction, including a FribourgOptions */
  // Object > AbstractExpression > CommandExpression > ComplementCommand
  class FribourgComplementCommand extends ComplementCommand {

    private FribourgOptions options;

    /* Constructor. Reads the arguments for 'fribourg', e.g. 'fribourg -m -m2'
     * and initialises a FribourgOptions accordingly. */
    public FribourgComplementCommand(List<Expression> args) throws IllegalArgumentException {
      super(args);
      // By default, all options are off, and they can be switched on by speci-
      // fying them on the command line. This is a convention that is followed
      // by the other existing algorithms in GOAL.
      boolean c=false, r2ifc=false, m=false, m2=false, b=false, macc=false, r=false, rr=false;
      for (Expression arg : args) {
        if (arg.toString().equals(cCode))          c     = true;
        else if (arg.toString().equals(r2ifcCode)) r2ifc = true;
        else if (arg.toString().equals(mCode))     m     = true;
        else if (arg.toString().equals(m2Code))    m2    = true;
        else if (arg.toString().equals(bCode))     b     = true;
        else if (arg.toString().equals(maccCode))  macc  = true;
        else if (arg.toString().equals(rCode))     r     = true;
        else if (arg.toString().equals(rrCode))    rr     = true;
        else throw new IllegalArgumentException("Unknown option: " + arg.toString());
      }
      if (m2 && !m) throw new IllegalArgumentException("Option -m2 requires option -m");
      options = new FribourgOptions();
      options.setC(c);
      options.setR2ifc(r2ifc);
      options.setM(m);
      options.setM2(m2);
      options.setB(b);
      options.setMacc(macc);
      options.setR(r);
      options.setRR(rr);
    }

    /* Return a FribourgConstruction initialised with our FribourgOptions */
    @Override // Abstract Method of ComplementCommand
    public ComplementConstruction<?, ?> getComplementConstruction(FSA in) {
      return new FribourgConstruction(in, options);
    }
  }

  /* The help text for 'fribourg' when typing 'goal help complement' */
  @Override // Method of interface CommandExtension
  public String getHelp() {
    String s;
    s =  pad(cCode)     + "Make input automaton complete if it is not complete. This option\n";
    s +=         "         generally DECREASES the performance of the algorithm.\n";
    s += pad(r2ifcCode) + "If the input automaton is complete, apply the optimisation of\n";
    s +=         "         deleting states whose rightmost component has colour 2.\n";
    s += pad(mCode)     + "Apply the optimisation of merging adjacent components.\n";
    s += pad(m2Code)    + "Apply the optimisation of reducing the number of components with\n";
    s +=         "         colour 2. This option requires the " + mCode + " option to be set.\n";
    s += pad(bCode)     + "Use the \"bracket notation\" for the state labels.\n";
    s += pad(maccCode)  + "Maximise the accpeting set of the input automaton.\n";
    s += pad(rCode)     + "Remove unreachable and dead states from the OUTPUT automaton.\n";
    s += pad(rrCode)    + "Remove unreachable and dead states from the INPUT automaton.";
    return s;
  }

  // Add suitable padding to the option codes so that the text is aligned
  private String pad(String code) {
    int gap = 7;
    int padding = gap - code.length();
    for (int i = 0; i < padding; i++) code += " ";
    code = "  " + code;
    return code;
  }

  @Override // Method of interface CommandExtension
  public CommandExpression parse(List<Expression> args) throws IllegalArgumentException {
    return new FribourgComplementCommand(args);
  }
}
