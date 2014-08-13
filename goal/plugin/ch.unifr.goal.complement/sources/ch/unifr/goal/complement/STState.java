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
  private Component m2;
  private Component m2Exclude;
  private int rightOffsetOfDisappearedM2;


  /* Constructor */
  public STState(int id) {
    super(id);
    components = new LinkedList<Component>();
    m2 = null;
    m2Exclude = null;
    rightOffsetOfDisappearedM2 = -1;
  }


  /* Component operations */
  public void addComponent(Component newComp) {
    addLeftmost(newComp);
  }
  public Component getComponent(int index) {
    return components.get(index);
  }
  public int numberOfComponents() {
    return components.size();
  }
  public void addComponentWithMerging(Component newComp) {
    if (components.size() == 0) addLeftmost(newComp);
    else {
      Component leftmostComp = getLeftmost();
      int newColor = newComp.getColor();
      switch (leftmostComp.getColor()) {
        case -1: case 0: // |X| -> {} ==> |X|{} (nothing can be merged in a {})
          addLeftmost(newComp);
          break;
        case 1: // () -> () ==> ()
          if (newColor == 1) mergeIn(newComp, 1);
          else addLeftmost(newComp);
          break;
        case 2: // () -> [] ==> []  AND  [] -> [] ==> []
          if (newColor == 1 || newColor == 2) mergeIn(newComp, 2);
          else addLeftmost(newComp);
      }
    }
  }
  private void mergeIn(Component newComp, int color) {
    Component leftmostComp = getLeftmost();
    StateSet mergedStates = new StateSet();
    mergedStates.addAll(leftmostComp.getStateSet());
    mergedStates.addAll(newComp.getStateSet());
    Component mergedComp = new Component(mergedStates, color);
    removeLeftmost();
    addLeftmost(mergedComp);
    // If the old leftmost had a role, then the new merged leftmost has it too
    if (leftmostComp == m2)        setM2(mergedComp);
    if (leftmostComp == m2Exclude) setM2Exclude(mergedComp);
  }

  
  /* M2 */
  public void setM2(Component comp) {
    m2 = comp;
  }
  public boolean hasM2() {
    return m2 != null;
  }
  public Component getM2() {
    return m2;
  }
  public Component getLeftNeighborOfM2() {
    if (isLeftmost(m2)) return getRightmost();
    else return components.get(components.indexOf(m2)-1);
  }


  /* M2Exclude */
  public void setM2Exclude(Component comp) {
    m2Exclude = comp;
  }
  public void setM2ExcludeLeftmost() {
    m2Exclude = getLeftmost();
  }
  public Component getM2Exclude() {
    return m2Exclude;
  }


  /* If this is state q, and M2(p) has disappeared */
  public void markM2Disappearance() {
    rightOffsetOfDisappearedM2 = components.size();
  }
  public boolean isM2Disappeared() {
    return rightOffsetOfDisappearedM2 != -1;
  }
  public int getDisappearedM2Position() {
    return components.size()-1-rightOffsetOfDisappearedM2;
  }


  /* Colours */
  public boolean hasOnlyColor0OrMinus1() {
    for (Component c : components)
      if (c.getColor() == 1 || c.getColor() == 2) return false;
    return true;
  }
  public boolean hasColor2() {
    for (Component c : components) if (c.getColor() == 2) return true;
    return false;
  }
  public int colorOfRightmostComponent() {
    return getRightmost().getColor();
  }

  /* State equivalence */
  public boolean equals(State otherState) {
    return this.getLabel().equals(otherState.getLabel()); 
  }

  /* Label */
  /* Create the label for the state that will be displayed in the box next to
   * each state in the GOAL GUI. The label has the form (({s0,s1},0),({s4},1)}.
   * It also servers for testing state equalitiy (see equals()). */
  public void makeLabelNormal() {
    // The char displayed in front of a state's ID (s or q)
    String prefix = Preference.getStatePrefix();
    String s = "(";
    if (rightOffsetOfDisappearedM2 != -1 && rightOffsetOfDisappearedM2 == numberOfComponents())
        s += "|,";
    for (Component c : components) {
      s += "({";
      for (State state : c.getStateSet()) s += prefix + state.getID() + ",";
      s = s.substring(0, s.length()-1); // Remove last superfluous comma
      s += "}," + c.getColor() + ")";
      if (c == m2) s += "*";
      s += ",";
      if (rightOffsetOfDisappearedM2 != -1 && components.indexOf(c) == components.size()-1-rightOffsetOfDisappearedM2)
        s += "|,";
    }
    s = s.substring(0, s.length()-1);   // Remove last superfluous comma
    s += ")";
    setLabel(s);
  }
  /* Label when the bracket notation option is on */
  public void makeLabelBrackets() {
    String prefix = Preference.getStatePrefix();
    String s = "(";
    if (rightOffsetOfDisappearedM2 != -1 && rightOffsetOfDisappearedM2 == numberOfComponents())
        s += "|,";
    for (Component c : components) {
      int color = c.getColor();
      String left = "", right = "";
      switch (color) {
        case -1:
          left = "^";
          right = "^";
          break;
        case 0:
          left = "{";
          right = "}";
          break;
        case 1:
          left = "(";
          right = ")";
          break;
        case 2:
          left = "[";
          right = "]";
      } 
      s += left;
      for (State state : c.getStateSet()) s += prefix + state.getID() + ",";
      s = s.substring(0, s.length()-1); // Remove last superfluous comma
      s += right;
      if (c == m2) s += "*";
      s += ",";
      if (rightOffsetOfDisappearedM2 != -1 && components.indexOf(c) == components.size()-1-rightOffsetOfDisappearedM2)
        s += "|,";
    }
    s = s.substring(0, s.length()-1);   // Remove last superfluous comma
    s += ")";
    setLabel(s);
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
    return state.getLabel().contains("},-1)") || state.getLabel().contains("^");
  }

  /* Small private interface to the components list */
  private void addLeftmost(Component c) {
    components.add(0, c);
  }
  private Component getLeftmost() {
    return components.get(0);
  }
  private void removeLeftmost() {
    components.remove(0);
  }
  private boolean isLeftmost(Component c) {
    return components.get(0) == c;
  }
  private Component getRightmost() {
    return components.get(components.size()-1);
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