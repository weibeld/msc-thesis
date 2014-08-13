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

  //private int indexOfM2;
  private Component m2;
  //private int indexOfM2Exclude;
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

  public Component getComponent(int index) {
    if (index == -1) System.out.println("index is -1: " + getLabel() + "index of m2: " + components.indexOf(m2));
    return components.get(index);
  }

  public int numberOfComponents() {
    return components.size();
  }

  /* Add a component as the new leftmost component of the state */
  public void addComponent(Component newComp) {
    addLeftmost(newComp);
  }

  public void addComponentWithMerging(Component newComp) {
    Component leftmostComp = getLeftmost();
    if (leftmostComp == null) {
      addLeftmost(newComp);
      return;
    }
    int newColor = newComp.getColor();
    switch (leftmostComp.getColor()) {
      case -1: case 0:
        addLeftmost(newComp);
        break;
      case 1:
        if (newColor == 1) mergeWithLeftmost(newComp, 1);
        else addLeftmost(newComp);
        break;
      case 2:
        if (newColor == 1 || newColor == 2) mergeWithLeftmost(newComp, 2);
        else addLeftmost(newComp);
    }
  }
  

  private void mergeWithLeftmost(Component newComp, int color) {
    Component leftmostComp = getLeftmost();
    StateSet mergedStates = new StateSet();
    mergedStates.addAll(leftmostComp.getStateSet());
    mergedStates.addAll(newComp.getStateSet());
    Component mergedComp = new Component(mergedStates, color);
    removeLeftmost();
    addLeftmost(mergedComp);
    if (isM2(leftmostComp)) setM2(mergedComp);
    if (isM2Exclude(leftmostComp)) setM2Exclude(mergedComp);
  }

  public Component getComponentWithColor0() {
    for (Component c : components)
      if (c.getColor() == 0) return c;
    return null;
  }

  // public Component getLeftNeighbor(Component comp) {
  //   int index = components.indexOf(comp);
  //   if (index == 0) return null;
  //   else return components.get(index-1);
  // }

  public boolean isLeftNeighborOfM2(Component comp) {
    if (isLeftmost(m2))
      if (isRightmost(comp)) return true;
      else return false;
    else {
      //System.out.println("index of m2: " + components.indexOf(m2));
      if (comp == getComponent(components.indexOf(m2) - 1)) return true;
      else return false;
    }
  }

  // public Component getColor0LeftOfM2() {
  //   if (indexOfM2 == 0) {
  //     if (getRightmost().getColor() == 0) return getRightmost();
  //     else return null;
  //   }
  //   else return components.get(indexOfM2 - 1);  // Must have colour 0
  // }

  public boolean isM2(Component comp) {
    if (comp == m2) return true;
    else return false;
  }

  // Set an existing component to M2
  public void setM2(Component comp) {
    m2 = comp;
  }

  // Set the leftmost component to M2
  public void setM2Leftmost() {
    m2 = getLeftmost();
  }

  public boolean hasM2() {
    return m2 != null;
  }

  public Component getM2() {
    if (m2 == null) return null;
    else return m2;
  }


  // Make the leftmost component the M2Exclude
  public void setM2ExcludeLeftmost() {
    m2Exclude = getLeftmost();
  }

  public void setM2Exclude(Component comp) {
    m2Exclude = comp;
  }

  public boolean isM2Exclude(Component comp) {
    if (comp == m2Exclude) return true;
    else return false;
  }

  public Component getM2Exclude() {
    if (m2Exclude == null) return null;
    else return m2Exclude;
  }


  // public void markM2() {
  //   markedM2RightOffset = components.size();
  // }

  // public boolean hasMarkedM2() {
  //   return markedM2RightOffset != -1;
  // }

  // public Component getMarkedM2() {
  //   return components.get(components.size() - markedM2RightOffset - 1);
  // }

  public void markM2Disappearance() {
    rightOffsetOfDisappearedM2 = components.size();
  }

  public boolean isM2Disappeared() {
    return rightOffsetOfDisappearedM2 != -1;
  }

  public int getDisappearedM2Position() {
    return components.size()-1-rightOffsetOfDisappearedM2;
  }

  // public boolean containsColor1Or2() {
  //   return !containsNoColor1And2();
  // }

  public boolean containsNoColor1And2() {
    for (Component c : components)
      if (c.getColor() == 1 || c.getColor() == 2) return false;
    return true;
  }

  public boolean containsColor2() {
    for (Component c : components) if (c.getColor() == 2) return true;
    return false;
  }

  public int colorOfRightmostComponent() {
    return getRightmost().getColor();
  }

  /* Tests if two STStates are equals by comparing their labels */
  public boolean equals(State otherState) {
    return this.getLabel().equals(otherState.getLabel()); 
  }

  public List<Component> getComponents() {
    return components;
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
      s += "}," + c.getColor() + ")";
      if (isM2(c)) s += "*";
      s += ",";
      if (rightOffsetOfDisappearedM2 != -1 && components.indexOf(c) == components.size()-1-rightOffsetOfDisappearedM2)
        s += "[],";
    }
    s = s.substring(0, s.length()-1);   // Remove last superfluous comma
    s += ")";
    setLabel(s);
  }

public String getLabel() {
    // The char displayed in front of a state's ID (s or q)
    String prefix = Preference.getStatePrefix();
    String s = "(";
    for (Component c : components) {
      s += "({";
      for (State state : c.getStateSet()) s += prefix + state.getID() + ",";
      s = s.substring(0, s.length()-1); // Remove last superfluous comma
      s += "}," + c.getColor() + ")";
      if (isM2(c)) s += "*";
      s += ",";
    }
    s = s.substring(0, s.length()-1);   // Remove last superfluous comma
    s += ")";
    return s;
  }
  // public void makeLabel() {
  //   String prefix = Preference.getStatePrefix();
  //   String s = "(";
  //   for (Component c : components) {
  //     Component c = getComponent(i);
  //     s += "({";
  //     for (State state : c.getStateSet()) s += prefix + state.getID() + ",";
  //     s = s.substring(0, s.length()-1); // Remove last superfluous comma
  //     s += "}," + c.getColor() + ")";
  //     if (isM2(c)) s += "*";
  //     s += ",";
  //   }
  //   s = s.substring(0, s.length()-1);   // Remove last superfluous comma
  //   s += ")";
  //   System.out.println(s);
  //   //this.setLabel(s);
  // }

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


  /* Small private interface to the components list */
  private void addLeftmost(Component c) {
    components.add(0, c);
  }
  private Component getLeftmost() {
    if (!components.isEmpty()) return components.get(0);
    else return null;
  }
  private void removeLeftmost() {
    if (!components.isEmpty()) components.remove(0);
  }
  private boolean isLeftmost(Component c) {
    return getLeftmost() == c;
  }
  private Component getRightmost() {
    if (!components.isEmpty()) return components.get(components.size()-1);
    else return null;
  }
  private boolean isRightmost(Component c) {
    return c == getRightmost();
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