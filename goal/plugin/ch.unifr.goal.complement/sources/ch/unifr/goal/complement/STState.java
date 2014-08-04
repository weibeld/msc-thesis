package ch.unifr.goal.complement;

/* Daniel Weibel, 25.07.2014 */

import java.util.List;
import java.util.LinkedList;
import org.svvrl.goal.core.aut.State;
import org.svvrl.goal.core.aut.fsa.FSAState;
import org.svvrl.goal.core.aut.StateSet;
import org.svvrl.goal.core.Preference;


/* A subset-tuple state (STState), i.e. a state of the output automaton of our
 * complementation construction. An STState consists of an ordered list (tuple)
 * of components (class Component). */
public class STState extends FSAState {

  List<Component> components;

  public STState(int id) {
    super(id);
    components = new LinkedList<Component>();
  }

  public Component getComponent(int index) {
    return components.get(index);
  }

  public int numberOfComponents() {
    return components.size();
  }

  /* Add a component as the leftmost element of the list of components of this state */
  public void addLeft(Component component) {
    components.add(0, component);
  }

  public boolean containsColor2() {
    for (Component c : components) if (c.getColor() == 2) return true;
    return false;
  }

  public int colorOfRightmostComponent() {
    return components.get(components.size()-1).getColor();
  }

  /* Create the label for the state that will be displayed in the rectangle
   * next to each state in the graphical GOAL. The label has the form
   * (({s0,s1},0),({s4},1)}. It also serves as a "signature" of a state for
   * allowing easy comparison if two states are the same. */
  public void makeLabel() {
    // The char displayed in front of a state's ID (s or q)
    //String prefix = Preference.getPreference(Preference.StatePrefixKey);
    String prefix = Preference.getStatePrefix();
    String s = "(";
    for (Component c : components) {
      s += "({";
      for (State state : c.getStateSet()) s += prefix + state.getID() + ",";
      s = s.substring(0, s.length()-1); // Remove last superfluous comma
      s += "}," + c.getColor() + "),";
    }
    s = s.substring(0, s.length()-1);   // Remove last superfluous comma
    this.setLabel(s);
  }

  /* The label of the "sink" state that is added to the constructed automaton
   * (output of stage 2) if it is not complete. */
  public void makeSinkLabel() {
    this.setLabel("sink");
  }

  /* Check if an STState is part of the upper part of the automaton, i.e. all
   * the components have color -1. It has to be done with a static method
   * because if we have an Automaton, we can only get State objects from it
   * and not STState objects, and we cannot cast a State to an STState. That is,
   * we cannot call a special STState instance method on a state returned by
   * Automaton. CAUTION: this way of testing for color -1 is very fragile. */
  public static boolean isFromUpperPart(State state) {
    return state.getLabel().contains("},-1)");
  }

  
  /* A component of a subset-tuple state. Consists of a set of states of the
   * input automaton, and a colour (-1, 0, 1, or 2). */
  public class Component {

    private StateSet stateSet;
    private int color;

    public Component(StateSet stateSet, int color) {
      this.stateSet = stateSet;
      this.color = color;
    }

    public Component(State state, int color) {
      stateSet = new StateSet();
      stateSet.add(state);
      this.color = color;
    }

    public StateSet getStateSet() {
      return stateSet;
    }

    public int getColor() {
      return color;
    }
  }

}