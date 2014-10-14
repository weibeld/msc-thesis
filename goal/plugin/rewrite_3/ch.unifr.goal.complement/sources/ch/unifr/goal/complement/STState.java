/*============================================================================*
 * Author: Daniel Weibel <daniel.weibel@unifr.ch>
 * Last modified: 14 Oct. 2014
 *============================================================================*/
package ch.unifr.goal.complement;

import java.util.List;
import java.util.LinkedList;
import org.svvrl.goal.core.aut.State;
import org.svvrl.goal.core.aut.StateSet;
import org.svvrl.goal.core.aut.fsa.FSAState;

/*----------------------------------------------------------------------------*
 * A subset-tuple state (STState), i.e. a state of the output automaton of the
 * Fribourg complementation construction. An STState consists of an ordered list
 * (tuple) of components (inner class Component).
 *----------------------------------------------------------------------------*/
public class STState extends FSAState {

  // The components of this state
  private List<Component> components;

  // Mark a state as accepting even though it has colour 2. For -m2 option only.
  private boolean hasAccFlag = false;

  // Constructor
  public STState(int id) {
    super(id);
    components = new LinkedList<Component>();
  }

  // Component operations
  public Component getComponent(int index) {
    return components.get(index);
  }
  public int numberOfComponents() {
    return components.size();
  }
  // Add component only if it's not empty (unless it's a EmptyColor2Component)
  public void addComponent(Component comp) {
    if (!comp.isEmpty() || comp.isEmptyColor2Component()) components.add(0, comp);
  }
  
  // Colour queries
  public boolean hasColor2() {
    for (Component c : components) if (c.color() == 2) return true;
    return false;
  }
  public boolean hasColor1() {
    for (Component c : components) if (c.color() == 1) return true;
    return false;
  }
  public boolean hasColor0() {
    for (Component c : components) if (c.color() == 0) return true;
    return false;
  }
  public boolean isRightmostColor2() {
    return components.get(components.size()-1).color() == 2;
  }
  // Checks if the components in a given state have colour -1. Since on the
  // calling end we have only access to a State object (instead of STState),
  // this test has to be made in this way via the label.
  public static boolean isFromUpperPart(State state) {
    return state.getLabel().contains("},-1)") || state.getLabel().contains("^");
  }

  // State equivalence
  public boolean equals(State other) {
    return this.getLabel().equals(other.getLabel()); 
  }

  // State label
  public void makeLabel(boolean bracketNotation) {
    String s = "(";
    for (Component c : components) s += c.print(bracketNotation) + ",";
    s = s.substring(0, s.length()-1); // Remove last superfluous comma
    s += ")";
    if (hasAccFlag) s += "*";
    setLabel(s);
  }
  // Label of a sink state
  public void makeSinkLabel() {
    this.setLabel("sink");
  }

  /*==========================================================================*
   * For the -m option
   *==========================================================================*/
  // Entry point: merge the components of a whole state
  public void mergeComponents() {
    // List is inverted for doing the []()->[] merge from right to left rather
    // than from left to right, which is easier.
    components = Util.invertList(mergeComps(Util.invertList(components)));
  }
  // Prolog-like merging of component list. Merge head with already merged tail.
  private LinkedList<Component> mergeComps(List<Component> list) {
    if (list.size() <= 1) return new LinkedList<Component>(list);
    return add(Util.head(list), mergeComps(Util.tail(list)));
  }
  // Add single component to left end of list
  private LinkedList<Component> add(Component c, LinkedList<Component> list) {
    Component h = list.get(0);
    if      (c.color() == 1 && h.color() == 1) list.set(0, merge(c, h, 1));
    else if (c.color() == 2 && h.color() == 2) list.set(0, merge(c, h, 2));
    else if (c.color() == 1 && h.color() == 2) list.set(0, merge(c, h, 2));
    else list.add(0, c);
    return list;
  }
  // Merge two components to one component with the specified color
  private Component merge(Component c1, Component c2, int color) {
    StateSet states = new StateSet();
    states.addAll(c1.stateSet());
    states.addAll(c2.stateSet());
    return new Component(states, color);
  }

  /*==========================================================================*
   * For the -m2 option
   *==========================================================================*/
  public boolean hasEmptyColor2() {
    return getEmptyColor2() != null;
  }
  // Precondition: hasEmptyColor2(), hasColor0(), and hasColor1() are true
  public Component getColor1ToMakeColor2() {
    return getColor1LeftOf(getColor0LeftOf(getEmptyColor2()));
  }
  public void setAccFlag() {
    hasAccFlag = true;
  }
  public boolean hasAccFlag() {
    return hasAccFlag;
  }
  public void removeEmptyColor2() {
    components.remove(getEmptyColor2());
  }
  private EmptyColor2Component getEmptyColor2() {
    for (Component c : components)
      if (c.isEmptyColor2Component()) return (EmptyColor2Component) c;
    return null;
  }
  private Component getColor1LeftOf(Component comp) {
    return getXLeftOf(1, comp);
  }
  private Component getColor0LeftOf(Component comp) {
    return getXLeftOf(0, comp);
  }
  // Returns null if there's no other component with color X
  private Component getXLeftOf(int color, Component comp) {
    Component cand = leftNeighborOf(comp);
    while (cand != comp) {
      if (cand.color() == color) return cand;
      cand = leftNeighborOf(cand);
    }
    return null;
  }
  private Component leftNeighborOf(Component comp) {
    int index = components.indexOf(comp);
    if (index > 0) return components.get(index-1);
    else return components.get(components.size()-1);
  }


  /*==========================================================================*
   * A component of an STState. A component consists of a set of states of the
   * input automaton, and a colour (-1, 0, 1, or 2).
   *==========================================================================*/
  public class Component {

    // Fields
    private StateSet stateSet;
    private int color;

    // Constructors
    public Component(StateSet stateSet, int color) {
      this.stateSet = stateSet;
      this.color = color;
    }
    public Component(State state, int color) {
      stateSet = new StateSet();
      stateSet.add(state);
      this.color = color;
    }

    // Getter and setter methods
    public StateSet stateSet() {
      return stateSet;
    }
    public boolean isEmpty() {
      return stateSet.isEmpty();
    }
    public int color() {
      return color;
    }
    public void setColor(int color) {
      this.color = color;
    }

    // Used for the -m2 option only
    public boolean isEmptyColor2Component() {
      return color == -2;
    }

    // String representation of this component, used for the state label
    public String print(boolean bracketNotation) {
      String s = "";
      if (bracketNotation) {
        if (color == -1)     s = "^" + Util.printStates(stateSet) + "^";
        else if (color == 0) s = "{" + Util.printStates(stateSet) + "}";
        else if (color == 1) s = "(" + Util.printStates(stateSet) + ")";
        else if (color == 2) s = "[" + Util.printStates(stateSet) + "]";
      }
      else s = "({" + Util.printStates(stateSet) + "}," + color + ")";
      return s;
    }
  }


  /*==========================================================================*
   * A special type of component representing an emptied 2-coloured component
   * for the -m2 option.
   *==========================================================================*/
  public class EmptyColor2Component extends Component {
    public EmptyColor2Component() {
      super(new StateSet(), -2);
    }
  }

}