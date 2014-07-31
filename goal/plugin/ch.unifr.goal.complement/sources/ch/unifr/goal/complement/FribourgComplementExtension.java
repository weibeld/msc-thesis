package ch.unifr.goal.complement;

/* Daniel Weibel, 25.07.2014 */

import java.util.List;
import org.svvrl.goal.cmd.AbstractCommandExtension;
import org.svvrl.goal.cmd.CommandExpression;
import org.svvrl.goal.cmd.ComplementCommand;
import org.svvrl.goal.cmd.Expression;
import org.svvrl.goal.core.aut.fsa.FSA;
import org.svvrl.goal.core.comp.ComplementConstruction;
import org.svvrl.goal.core.Properties;


/* The 'fribourg' extension to the 'goal complement' command on the command line
 * Object > AbstractCommandExtension */
public class FribourgComplementExtension extends AbstractCommandExtension {

  /* A class that is able to create a FribourgConstruction, initialised with
   * a FribourgOptions according to the command line arguments.
   * Object > AbstractExpression > CommandExpression > ComplementCommand */
  class FribourgComplementCommand extends ComplementCommand {
    // A FribourgOptions adapted according to the arguments on the command line
    private FribourgOptions options;

    /* Constructor. Main task is to initialise the FribourgOptions object */
    public FribourgComplementCommand(List<Expression> args) throws IllegalArgumentException {
      super(args);
      // If an option is specified on the command line, the value for that
      // option is the inverse of the default value of that option. For options
      // that are not specified, the value is the default value.
      Properties props = new Properties();
      boolean c = FribourgOptions.getMakeCompleteDefault();
      boolean r2 = FribourgOptions.getDelRight2Default();
      for (Expression arg : args) {
        if (arg.toString().equals("-c")) {
          c = !FribourgOptions.getMakeCompleteDefault();
          props.setProperty(FribourgOptions.getMakeCompleteKey(), c);
        }
        else if (arg.toString().equals("-r2")) {
          r2 = !FribourgOptions.getDelRight2Default();
          props.setProperty(FribourgOptions.getDelRight2Key(), r2);
        }
        else
          throw new IllegalArgumentException("Unknown option: " + arg.toString());
      }
      if (r2 && !c) throw new IllegalArgumentException("Can only apply rightmost-colour-2 optimisation if the input automaton is ensured to be complete.");
      // The FribourgOptions constructor sets the default values for the options
      // that are not present in the argument.
      options = new FribourgOptions(props);
    }

    /* Create a FribourgConstruction with FribourgOptions */
    @Override // Abstract Method of ComplementCommand
    public ComplementConstruction<?, ?> getComplementConstruction(FSA in) {
      return new FribourgConstruction(in, options);
    }
  }

  /* The text below 'ARGUMENTS FOR fribourg' when typing 'goal help complement' */
  @Override // Method of interface CommandExtension
  public String getHelp() {
    String s;
    s =  "  -c    Make input automaton complete if it is not complete.\n";
    s += "  -r2   Apply optimisation of deleting states whose rightmost component has\n";
    s += "        colour 2. Requires option -c to be set.";
    return s;
  }

  @Override // Method of interface CommandExtension
  public CommandExpression parse(List<Expression> args) throws IllegalArgumentException {
    return new FribourgComplementCommand(args);
  }
}
