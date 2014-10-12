/*============================================================================*
 * Author: Daniel Weibel <daniel.weibel@unifr.ch>
 * Last modified: 5 Oct. 2014
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

  // The state's components. The order of the list reflects the order of the
  // components in the state. The first element in list is the leftmost com-
  // ponent, and the last element int the list is the rightmost component.
  private List<Component> components;

  /* Constructor */
  public STState(int id) {
    super(id);
    components = new LinkedList<Component>();
  }

  public boolean hasEmptyC2Component() {
    return getEmptyC2Component() != null;
  }
  public void removeEmptyC2Component() {
    components.remove(getEmptyC2Component());
  }
  public Component getC1ToBeMadeC2() {
    if (!hasC1Components()) return null;
    EmptyC2Component ec2 = getEmptyC2Component();
    Component c0 = getC0ToLeftOf(ec2);
    if (c0 != null) return getC1ToLeftOf(c0);
    return getC1ToLeftOf(getC1ToLeftOf(ec2));
  }
  private EmptyC2Component getEmptyC2Component() {
    for (Component c : components)
      if (c.isEmptyC2Component()) return (EmptyC2Component) c;
    return null;
  }
  private Component getC1ToLeftOf(Component comp) {
    Component cand = leftNeighbor(comp);
    while (cand != comp) {
      if (cand.color() == 1) return cand;
      cand = leftNeighbor(cand);
    }
    if (cand.color() == 1) return cand; // cand == comp
    return null;
  }
  private Component getC0ToLeftOf(Component comp) {
    Component cand = leftNeighbor(comp);
    while (cand != comp) {
      if (cand.color() == 0) return cand;
      cand = leftNeighbor(cand);
    }
    if (cand.color() == 0) return cand; // cand == comp
    return null;
  }
  private Component leftNeighbor(Component comp) {
    int index = components.indexOf(comp);
    if (index > 0) return components.get(index-1);
    else return components.get(components.size()-1);
  }

  private boolean hasC1Components() {
    for (Component c : components)
      if (c.color() == 1) return true;
    return false;
  }

  /* Component operations */
  public void addComponent(Component newComp) {
    if (!newComp.isEmpty() || newComp.isEmptyC2Component()) addLeftmost(newComp);
  }
  public Component getComponent(int index) {
    return components.get(index);
  }
  public int numberOfComponents() {
    return components.size();
  }

  //------------------
  // For the merging optimisation (-m)
  // Apply all possible mergings on the components of the state, i.e. ()()->(),
  // []()->[], and [][]->[]. The inversion of the list is for doing the []()->[]
  // merge more conveniently.
  public void mergeComponents() {
    components = Util.invertList(mergeComps(Util.invertList(components)));
  }
  // Prolog-like merging. Merge head with already merged tail
  private LinkedList<Component> mergeComps(List<Component> list) {
    if (list.size() <= 1) return new LinkedList<Component>(list);
    return add(Util.head(list), mergeComps(Util.tail(list)));
  }
  // Push new component on left end of list, if possible, merge with list head
  private LinkedList<Component> add(Component c, LinkedList<Component> list) {
    Component h = list.get(0);
    if      (c.color() == 1 && h.color() == 1) list.set(0, merge(c, h, 1));
    else if (c.color() == 2 && h.color() == 2) list.set(0, merge(c, h, 2));
    else if (c.color() == 1 && h.color() == 2) list.set(0, merge(c, h, 2));
    else list.add(0, c);
    return list;
  }
  // Merge two components into one
  private Component merge(Component c1, Component c2, int color) {
    StateSet states = new StateSet();
    states.addAll(c1.stateSet());
    states.addAll(c2.stateSet());
    Component mergedComp = new Component(states, color);
    // if (c1.isStar() || c2.isStar()) mergedComp.setStar();
    // if (c1.isSuppressed() || c2.isSuppressed()) mergedComp.setSuppressed();
    return mergedComp;
  }
  //------------------
  
  // public void addComponentWithMerging(Component newComp) {
  //   if (components.size() == 0) addLeftmost(newComp);
  //   else {
  //     Component leftmostComp = getLeftmost();
  //     int newColor = newComp.color();
  //     switch (leftmostComp.color()) {
  //       // |X| -> {} ==> |X|{} (nothing can be merged in a {})
  //       case -1: case 0:
  //         addLeftmost(newComp);
  //         break;
  //       case 1:
  //         // () -> ().. ==> ()..
  //         if (newColor == 1) mergeIn(newComp, 1);
  //         // [] -> ().. ==> []..
  //         else if (newColor == 2) {
  //           mergeIn(newComp, 2);
  //           // [] -> ()[].. ==> [][].. ==> []
  //           if (components.size() > 1 && components.get(1).color() == 2) {
  //             leftmostComp = getLeftmost();
  //             removeLeftmost();
  //             mergeIn(leftmostComp, 2);
  //           }
  //         }
  //         else addLeftmost(newComp);
  //         break;
  //       case 2:
  //         // [] -> [].. ==> []..
  //         if (newColor == 2) mergeIn(newComp, 2);
  //         else addLeftmost(newComp);
  //     }
  //   }
  // }
  // private void mergeIn(Component newComp, int color) {
  //   Component leftmostComp = getLeftmost();
  //   StateSet mergedStates = new StateSet();
  //   mergedStates.addAll(leftmostComp.stateSet());
  //   mergedStates.addAll(newComp.stateSet());
  //   Component mergedComp = new Component(mergedStates, color);
  //   removeLeftmost();
  //   addLeftmost(mergedComp);
  //   // If one of the merged comp. had a role, then the new leftmost has it too
  //   if (leftmostComp == m2 || newComp == m2) setM2(mergedComp);
  //   if (leftmostComp == m2Exclude || newComp == m2Exclude) setM2Exclude(mergedComp);
  // }

  
  // /* M2 */
  // public void setM2(Component comp) {
  //   m2 = comp;
  // }
  // public boolean hasM2() {
  //   return m2 != null;
  // }
  // public Component getM2() {
  //   return m2;
  // }
  // public Component getLeftNeighborOfM2() {
  //   if (isLeftmost(m2)) return getRightmost();
  //   else return components.get(components.indexOf(m2)-1);
  // }


  // /* M2Exclude */
  // public void setM2Exclude(Component comp) {
  //   m2Exclude = comp;
  // }
  // public void setM2ExcludeLeftmost() {
  //   m2Exclude = getLeftmost();
  // }
  // public Component getM2Exclude() {
  //   return m2Exclude;
  // }


  // /* If this is state q, and M2(p) has disappeared */
  // public void markM2Disappearance() {
  //   rightOffsetOfDisappearedM2 = components.size();
  // }
  // public boolean isM2Disappeared() {
  //   return rightOffsetOfDisappearedM2 != -1;
  // }
  // public int getDisappearedM2Position() {
  //   return components.size()-1-rightOffsetOfDisappearedM2;
  // }


  /* Colours */
  public boolean hasOnlyColor0OrMinus1() {
    for (Component c : components)
      if (c.color() == 1 || c.color() == 2) return false;
    return true;
  }
  public boolean hasColor2() {
    for (Component c : components) if (c.color() == 2) return true;
    return false;
  }
  public int colorOfRightmostComponent() {
    return getRightmost().color();
  }
  public int colorOfSecondLeftmostComponent() {
    if (components.size() > 1) return components.get(1).color();
    else return 999;
  }

  /* State equivalence */
  public boolean equals(State otherState) {
    return this.getLabel().equals(otherState.getLabel()); 
  }

  /* Label */
  /* Create the label for the state that will be displayed in the box next to
   * each state in the GOAL GUI. The label has the form (({s0,s1},0),({s4},1)).
   * It also servers for testing state equalitiy (see equals()). */
  public void makeLabel1() {
    String s = "(";
    for (Component c : components) s += c.print1() + ",";
    s = s.substring(0, s.length()-1);   // Remove last superfluous comma
    s += ")";
    setLabel(s);
  }
  /* Label when the bracket notation option is on */
  public void makeLabel2() {
    String s = "(";
    for (Component c : components) s += c.print2() + ",";
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
    public boolean isEmptyC2Component() {
      return color == -2;
    }
    public String print1() {
      String s = "({" + Util.printStates(stateSet) + "}," + color + ")";
      //if (isStar()) s += "*";
      return s;
    }
    public String print2() {
      String s = "";
      if (color == -1)     s = "^" + Util.printStates(stateSet) + "^";
      else if (color == 0) s = "{" + Util.printStates(stateSet) + "}";
      else if (color == 1) s = "(" + Util.printStates(stateSet) + ")";
      else if (color == 2) s = "[" + Util.printStates(stateSet) + "]";
      //if (isStar()) s += "*";
      return s;
    }

    // For the colour 2 reduction optimisation (-m2)
    // private boolean star;
    // private boolean suppressedColor2;

    // public boolean isStar() {
    //   return star;
    // }
    // public boolean isSuppressed() {
    //   return suppressedColor2;
    // }

    // public void setStar() {
    //   star = true;
    // }
    // public void setSuppressed() {
    //   suppressedColor2 = true;
    // }
  }

  public class EmptyC2Component extends Component {
    public EmptyC2Component() {
      super(new StateSet(), -2);
    }
    public String print1() {
      return "|";
    }
    public String print2() {
      return "|";
    }
  }

}