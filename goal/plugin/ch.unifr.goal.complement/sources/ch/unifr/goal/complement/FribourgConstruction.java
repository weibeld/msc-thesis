package ch.unifr.goal.complement;

/* Contains the actual complementation construction */

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

import ch.unifr.goal.complement.STState;
import ch.unifr.goal.complement.STState.Component;
import java.util.Set;
import java.util.HashSet;
import java.util.Arrays;


/* Class hierarchy: Object > AbstractAlgorithm > AbstractControllableAlgorithm >
 * AbstractEditableAlgorithm > ComplementConstruction */
public class FribourgConstruction extends ComplementConstruction<FSA, FSA> {

  /* Holding the complement automaton if complement() has already been
   * executed before. */
  private FSA complement = null;


  /* Constructor
   * in is the input automaton to this construction. */
  public FribourgConstruction(FSA in) {
    super(in);
    if (!OmegaUtil.isNBW(in))
      throw new IllegalArgumentException(Message.onlyForFSA(BuchiAcc.class));
  }


  /* Method from interface EditableAlgorithm */
  @Override
  public Editable getIntermediateResult() {
    return complement;
  }


  /* Abstract method from ComplementConstruction. Calls the method construction
   * which is the entry point to the complementation construction. */
  @Override
  public FSA complement() {
    if (complement != null) return complement;
    fireReferenceChangedEvent(); // Method of AbstractControllableAlgorithm
    // getInput: method of ComplementConstruction. Returns automaton to be
    // complemented.
    complement = construction(getInput().clone());
    return complement;
  }



  private FSA construction(FSA in) {

    // if (true) {
    // in.setCompleteTransitions(false);
    // in.updateTransitionDisplay();
    // return in;
    // }

    // The input automaton: alphabet, initial state, and accepting states
    String[] inAlphabet = in.getAlphabet();
    State inInitState = in.getInitialState();
    BuchiAcc inAccStates = (BuchiAcc) in.getAcc();

    // Make the input automaton complete before starting the construction
    if (!isComplete(in)) makeComplete(in);

    // The output automaton
    FSA out = new FSA(AlphabetType.CLASSICAL, Position.OnTransition);
    out.expandAlphabet(inAlphabet);

    // List of references to all the states in the output automaton so far in
    // the construction. We need this to be able to loop through all the states
    // in our automaton in order to determine if a given state already exists.
    StateSet outStates = new StateSet();

    BuchiAcc outAccStates = new BuchiAcc();

    // We take care of giving IDs to the states in the out automaton ourselves
    int id = 0;

    // The subset-tuple states to process
    StateSet pendingSTStates = new StateSet();

    for (int i = 1; i <= 2; i++) {

      if (i == 1) {
      // Adding the initial state
      STState outInitState = new STState(id++);
      outInitState.addLeft(outInitState.new Component(inInitState, -1));
      outInitState.makeLabel();
      out.addState(outInitState);
      out.setInitialState(outInitState);
      pendingSTStates.add(outInitState);
      outStates.add(outInitState);
      }
      else if (i == 2) {
        pendingSTStates = outStates.clone();
      }
      
      while (!pendingSTStates.isEmpty()) {

        STState currentSTState = (STState) pendingSTStates.pollFirst();

        for (String symbol : inAlphabet) {
          // The "symbol"-successor of the current subset-tuple state
          STState succSTState = new STState(id);

          StateSet seenFSAStates = new StateSet();

          // Iterate through the components of this state from right to left
          for (int j = currentSTState.numberOfComponents() - 1; j >= 0; j--) {
            Component currentComponent = currentSTState.getComponent(j);
            StateSet succFSAStates = in.getSuccessors(currentComponent.getStateSet(), symbol);
            succFSAStates.removeAll(seenFSAStates);
            seenFSAStates.addAll(succFSAStates);
            StateSet accFSAStates = new StateSet();
            StateSet naccFSAStates = new StateSet();
            for (State state : succFSAStates) {
              if (inAccStates.contains(state)) accFSAStates.add(state);
              else naccFSAStates.add(state);
            }

            for (int k = 0; k < 2; k++) {
              StateSet currentSet = new StateSet();
              if (k == 0) currentSet = accFSAStates;
              else if (k == 1) currentSet = naccFSAStates;
              if (currentSet.isEmpty()) continue;
              int color = -1;
              if (i == 1) color = -1;
              else if (i == 2) {
                int predColor = currentComponent.getColor();
                if (currentSTState.containsColor2()) {
                  if (predColor == 0 && k != 0) color = 0;
                  else if (predColor == 2) color = 2;
                  else color = 1;
                }
                else {
                  if ((predColor == 0 || predColor == -1) && k != 0) color = 0;
                  else color = 2;
                }
              }
              succSTState.addLeft(succSTState.new Component(currentSet, color));
            }

          }

          // Does succSTState already exist in the automaton?
          succSTState.makeLabel();
          boolean alreadyExists = false;
          for (State existingSTState : outStates) {
            if (succSTState.getLabel().equals(existingSTState.getLabel())) {
              succSTState = (STState) existingSTState;
              alreadyExists = true;
              break;
            }
          }
          if (!alreadyExists) {
            // Optimisation of Sec. IV.E of the paper. If the rightmost component
            // has colour 2, don't create the state.
            if (i == 2 && succSTState.colorOfRightmostComponent() == 2) continue;
            out.addState(succSTState);
            outStates.add(succSTState);
            pendingSTStates.add(succSTState);
            id++;
            if (i == 2 && !succSTState.containsColor2()) {
              outAccStates.add(succSTState);
            }
          }
          out.createTransition(currentSTState, succSTState, symbol);
        }
      }
    }

    out.setAcc(outAccStates);

    return out;



    // FSA out = new FSA(AlphabetType.CLASSICAL, Position.OnTransition);

    // States
    // FSAState q0 = out.createState();
    // FSAState q1 = out.createState();
    // FSAState q2 = out.createState();
    // FSAState q3 = new FSAState(8);
    // out.addState(q3);

    // Initial state
    // out.setInitialState(q0);

    // Accepting states
    // BuchiAcc acc = new BuchiAcc();
    // acc.add(q0);
    // out.setAcc(acc);

    // out.expandAlphabet(in.getAlphabet());

    // Transitions and alphabet
    // out.createTransition(q0,q0,"a");
    // out.createTransition(q0,q0,"b");
    // out.createTransition(q0,q1,"a");
    // out.createTransition(q0,q1,"b");
    // out.createTransition(q1,q1,"b");
    // out.createTransition(q1,q2,"a");
    // out.createTransition(q2,q2,"a");
    // out.createTransition(q2,q2,"b");
    // out.createTransition(q2,q3,"a");

    // String str = "alphabet of input aut. {";
    // for (String symbol : in.getAlphabet()) {
    //   str += symbol + ",";
    // }
    // str += "}";

    // q0.setLabel("" + str);

    // return out;
  }

  /* Checks if the automaton passed as argument is complete (i.e. at least one
   * transition for every symbol of the alphabet out of every state), or not. */
  private boolean isComplete(FSA a) {
    for (State s : a.getStates()) {
      if (a.getSymbolsFromState(s).size() != a.getAlphabet().length) return false;
    }
    return true;
  }

  /* Makes the incomplete automaton passed as argument complete by creating an
   * additional non-accepting "dead state" and adding all missing transitions
   * to this state. Assumes that the automaton in the argument is incomplete. */
  private void makeComplete(FSA a) {
    FSAState deadState = a.createState();
    Set<String> alphabet = new HashSet<String>(Arrays.asList(a.getAlphabet()));
    for (State state : a.getStates()) {
      Set<String> should = new HashSet<String>(alphabet);
      Set<String> is = new HashSet<String>(a.getSymbolsFromState(state));
      should.removeAll(is);
      for (String symbol : should) a.createTransition(state, deadState, symbol);
    }
    for (String symbol : alphabet) a.createTransition(deadState, deadState, symbol);
  }

}
