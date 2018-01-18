/*============================================================================*
 * Command line command "fribourg" for the command line interface of GOAL
 * (gc complement -m fribourg). Works together with the
 * "ComplementConstructionInterface" extension in plugin.xml.
 *
 * Copyright (c) 2014-2015 Daniel Weibel <daniel.weibel@unifr.ch>
 *============================================================================*/

package ch.unifr.goal.complement;

import org.svvrl.goal.cmd.AbstractComplementConstructionInterface;
import java.util.List;
import org.svvrl.goal.cmd.Context;
import org.svvrl.goal.cmd.Expression;
import org.svvrl.goal.core.Properties;
import org.svvrl.goal.cmd.EvaluationException;

// AbstractComplementConstructionInterface implements ComplementConstructionInterface
public class FribourgComplementInterface extends AbstractComplementConstructionInterface {

  // The command line codes for all the options for 'complement -m fribourg ...'
  private final String mCode     = "-m1";   // Merge components
  private final String m2Code    = "-m2";   // Single 2-coloured component
  private final String r2ifcCode = "-r2c";  // Remove rightmost colour 2 if complete
  private final String cCode     = "-c";    // Make input automaton complete
  private final String maccCode  = "-macc"; // Maximise accepting set of input
  private final String rCode     = "-r";    // Remove unreachable and dead from OUTPUT
  private final String rrCode    = "-rr";   // Remove unreachable and dead from INPUT
  private final String bCode     = "-b";    // Use the bracket notation
  
  // Parsing of the options string from the command line
  public Properties getOptions(Context context, List<Expression> args) throws EvaluationException {
    // By default, all options are off, and they can be switched on by speci-
    // fying them on the command line. This is a convention that is followed
    // by the other existing algorithms in GOAL.
    boolean c=false, r2ifc=false, m=false, m2=false, b=false, macc=false, r=false, rr=false;
    for (Expression a : args) { // args contains e.g. ["-m", "-macc", "-r"]
      String arg = a.toString();
      if (arg.equals(cCode))          c     = true;
      else if (arg.equals(r2ifcCode)) r2ifc = true;
      else if (arg.equals(mCode))     m     = true;
      else if (arg.equals(m2Code))    m2    = true;
      else if (arg.equals(bCode))     b     = true;
      else if (arg.equals(maccCode))  macc  = true;
      else if (arg.equals(rCode))     r     = true;
      else if (arg.equals(rrCode))    rr    = true;
      else throw new EvaluationException("Unknown option: " + arg);
    }
    if (m2 && !m) throw new EvaluationException("Option -m2 requires option -m1");
    FribourgOptions options = new FribourgOptions();
    options.setC(c);
    options.setR2ifc(r2ifc);
    options.setM(m);
    options.setM2(m2);
    options.setB(b);
    options.setMacc(macc);
    options.setR(r);
    options.setRR(rr);
    return options;
  }

  // The help text in 'goal help complement'
  @Override
  public String getHelp() {
    String s = "";
    s += pad(r2ifcCode) + "Apply R2C optimisation (remove states with 2-coloured rightmost comp.)\n";
    s += pad(mCode)     + "Apply M1 optimisation (merging of adjacent components)\n";
    s += pad(m2Code)    + "Apply M2 optimisation (single 2-coloured component), requires " + mCode + "\n";
    s += pad(cCode)     + "Make input automaton complete\n"; 
    s += pad(maccCode)  + "Maximise accepting set of input automaton\n";
    s += pad(rCode)     + "Remove unreachable and dead states from OUTPUT automaton\n";
    s += pad(rrCode)    + "Remove unreachable and dead states from INPUT automaton\n";
    s += pad(bCode)     + "Use the \"bracket notation\" for state labels";
    return s;
  }

  // Add suitable padding to the option codes so that the text is aligned
  private String pad(String code) {
    int gap = 7;
    int padding = gap - code.length();
    for (int i = 0; i < padding; i++) code += " ";
    return "  " + code;
  }
}