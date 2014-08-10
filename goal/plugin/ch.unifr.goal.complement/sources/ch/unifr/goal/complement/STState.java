package ch.unifr.goal.complement;

import java.util.List;
import java.util.LinkedList;
import org.svvrl.goal.core.aut.State;
import org.svvrl.goal.core.aut.fsa.FSAState;
import org.svvrl.goal.core.aut.StateSet;
import org.svvrl.goal.core.Preference;

/*----------------------------------------------------------------------------*
 * A subset-tuple state (STState), i.e. a state of the output automaton of the
 * Fribourg complementation construction. An STState consists of an ordered list
 * (tuple) of components (inner class Component).
 * Daniel Weibel, 25.07.2014
 *----------------------------------------------------------------------------*/
public class STState extends FSAState {

  // The state's components. The order of the list reflects the order of the
  // components in the state. The first element in list is the leftmost com-
  // ponent, and the last element int the list is the rightmost component.
  private List<Component> components;

  /* Constructor */
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

  /* Add a component as the new leftmost component of the state */
  public void addComponent(Component component) {
    components.add(0, component);
  }

  public boolean containsColor2() {
    for (Component c : components) if (c.getColor() == 2) return true;
    return false;
  }

  public int colorOfRightmostComponent() {
    return components.get(components.size()-1).getColor();
  }

  /* Tests if two STStates are equals by comparing their labels */
  public boolean equals(State otherState) {
    return this.getLabel().equals(otherState.getLabel()); 
  }

  /* Create the label for the state that will be displayed in the box next to
   * each state in the GOAL GUI. The label has the form (({s0,s1},0),({s4},1)}.
   * It also servers for testing state equalitiy (see equals()). */
  public void makeLabel() {
    // The char displayed in front of a state's ID (s or q)
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

  /* The label of the sink state that is added to the upper part of the
   * automaton if it is not complete. */
  public void makeSinkLabel() {
    this.setLabel("sink");
  }

  /* Check if an STState is part of the upper part of the automaton, i.e. all
   * the components have color -1. It has to be done indirectly via a State
   * object (and not STState) because the Automaton class returns only State
   * objects. That's also why we have to make the test via the label. */
  public static boolean isFromUpperPart(State state) {
    return state.getLabel().contains("},-1)");
  }

  /*--------------------------------------------------------------------------*
   * A component of an STState. A component consists of a set of states of the
   * input automaton, and a colour (-1, 0, 1, or 2).
   *--------------------------------------------------------------------------*/
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