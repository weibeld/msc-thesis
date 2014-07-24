package ch.unifr.goal.complement;

import org.svvrl.goal.core.Editable;
import org.svvrl.goal.core.Message;
import org.svvrl.goal.core.aut.BuchiAcc;
import org.svvrl.goal.core.aut.OmegaUtil;
import org.svvrl.goal.core.aut.State;
import org.svvrl.goal.core.aut.StateSet;
import org.svvrl.goal.core.aut.fsa.FSA;
import org.svvrl.goal.core.comp.ComplementConstruction;

import org.svvrl.goal.core.aut.AlphabetType;
import org.svvrl.goal.core.aut.Position;
import org.svvrl.goal.core.aut.fsa.FSAState;

import java.util.Set;
import java.util.LinkedList;
import java.util.List;

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

  // Add a component to the left end of the tuple of this state
  public void addLeft(Component component) {
    components.add(0, component);
  }

  // Create the label for the state that will be displayed in the rectangle
  // next to each state in the graphical GOAL. The label has the form
  // (({s0,s1},0),({s4},1)}
  public void makeLabel() {
    String s = "(";
    for (Component c : components) {
      s += "({";
      for (State state : c.getStateSet()) s += "s" + state.getID() + ",";
      s = s.substring(0, s.length()-1); // Remove last superfluous comma
      s += "}," + c.getColor() + "),";
    }
    s = s.substring(0, s.length()-1); // Remove last superfluous comma
    this.setLabel(s);
  }



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