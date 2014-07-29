package ch.unifr.goal.complement;

/* Contains the complementation construction */

/* Daniel Weibel, 25.07.2014 */

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
import ch.unifr.goal.complement.FribourgOptions;
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
   * which contains the complementation construction. */
  @Override
  public FSA complement() {
    if (complement != null) return complement;
    fireReferenceChangedEvent(); // Method of AbstractControllableAlgorithm
    // getInput: method of ComplementConstruction. Returns automaton to be complemented.
    complement = construction(getInput().clone());
    return complement;
  }


  private FSA construction(FSA in) {

    // The input automaton: alphabet, initial state, and accepting states
    String[] inAlphabet = in.getAlphabet();
    State inInitState = in.getInitialState();
    BuchiAcc inAccStates = (BuchiAcc) in.getAcc();

    // Make the input automaton complete before starting the construction
    if (!isComplete(in)) makeComplete(in);

    // The output automaton: alphabet and accepting states
    FSA out = new FSA(AlphabetType.CLASSICAL, Position.OnTransition);
    out.expandAlphabet(inAlphabet);
    BuchiAcc outAccStates = new BuchiAcc();

    // We take care of giving IDs to the states in the output automaton ourselves
    int id = 0;

    // The states of the output automaton whose successors we have to determine
    StateSet pendingSTStates = new StateSet();

    // We do the same construction twice with slight differences. Iteration 1
    // constructs the upper part, and iteration 2 the lower part of the automaton.
    for (int i = 1; i <= 2; i++) {
      // Adding the initial state
      if (i == 1) {
        STState outInitState = new STState(id++);
        outInitState.addLeft(outInitState.new Component(inInitState, -1));
        outInitState.makeLabel();
        out.addState(outInitState);
        out.setInitialState(outInitState);
        pendingSTStates.add(outInitState);
      }
      // In the second stage we process all the states of the upper automaton again
      else if (i == 2) pendingSTStates = new StateSet(out.getStates());
      
      while (!pendingSTStates.isEmpty()) {
        // The state whose successors we are going to determine
        STState currentSTState = (STState) pendingSTStates.pollFirst();
        for (String symbol : inAlphabet) {
          // The current state's successor that we are going to construct
          STState succSTState = new STState(id);
          // The input automaton states that have already occured in the right-to-left traversal
          StateSet seenFSAStates = new StateSet();
          // Iterate through the components of the current state from right to left
          for (int j = currentSTState.numberOfComponents()-1; j >= 0; j--) {
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
            // One component in the current state can give birth to two new components
            // (an accepting and a non-accepting one) in the successor state. In the
            // following loop, the first iteration treats the accepting set and the
            // second one the non-accepting set.
            for (int k = 0; k < 2; k++) {
              StateSet currentSet = new StateSet();
              if (k == 0) currentSet = accFSAStates;
              else if (k == 1) currentSet = naccFSAStates;
              if (currentSet.isEmpty()) continue;
              int color = -1;
              if (i == 1) color = -1;   // Stage 1 (upper part of automaton)
              else if (i == 2) {        // Stage 2 (lower part of automaton)
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
              // Add the component as the new leftmost component of the successor state
              succSTState.addLeft(succSTState.new Component(currentSet, color));
            }
          } // End of iterating through all the components of the current state

          // Does succSTState already exist in the automaton? The label of a state
          // serves as a state's "signature" that is used for comparing if two
          // states are the same
          succSTState.makeLabel();
          boolean alreadyExists = false;
          for (State existingState : out.getStates()) {
            if (succSTState.getLabel().equals(existingState.getLabel())) {
              succSTState = (STState) existingState;
              alreadyExists = true;
              break;
            }
          }
          if (!alreadyExists) {
            // Optimisation of deleting states with rightmost component colour 2
            // The 'continue' jumps to the head of the loop through the alphabet
            if (i == 2 && succSTState.colorOfRightmostComponent() == 2) continue;
            out.addState(succSTState);
            pendingSTStates.add(succSTState);
            id++;
            // Set states containing no colour 2 as accepting states
            if (i == 2 && !succSTState.containsColor2()) outAccStates.add(succSTState);
          }
          out.createTransition(currentSTState, succSTState, symbol);
        }
      }
    }

    out.setAcc(outAccStates);

    return out;

    /* Automaton API summary (see API of class Automaton or FSA) */
    // States
    // FSAState q0 = out.createState();
    // FSAState q1 = new FSAState(8);
    // out.addState(q1);

    // Initial state
    // out.setInitialState(q0);

    // Accepting states
    // BuchiAcc acc = new BuchiAcc();
    // acc.add(q1);
    // out.setAcc(acc);
  
    // Alphabet
    // out.expandAlphabet(in.getAlphabet());

    // Transitions
    // out.createTransition(q0,q0,"a");
    // out.createTransition(q0,q1,"b");
    // out.createTransition(q1,q1,"a");
    // out.createTransition(q1,q1,"b");

    // Labels
    // q0.setLabel("s" + q0.getID());
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
