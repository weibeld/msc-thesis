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

    // The input automaton: alphabet, initial state, and accepting states
    String[] inAlphabet = in.getAlphabet();
    State inInitState = in.getInitialState();
    BuchiAcc inAccStates = (BuchiAcc) in.getAcc();

    // The output automaton
    FSA out = new FSA(AlphabetType.CLASSICAL, Position.OnTransition);

    // List of references to all the states in the output automaton so far in
    // the construction. We need this to be able to loop through all the states
    // in our automaton in order to determine if a given state already exists.
    StateSet outStates = new StateSet();

    // We take care of giving IDs to the states in the out automaton ourselves
    int id = 0;

    /* Stage 1: non-deterministic Büchi automaton to deterministic interim
     * automaton */

    // The subset-tuple states to process
    StateSet pendingSTStates = new StateSet();

    // Adding the initial state
    STState outInitState = new STState(id++);
    outInitState.addLeft(outInitState.new Component(inInitState, -1));
    outInitState.makeLabel();
    out.addState(outInitState);
    out.setInitialState(outInitState);
    pendingSTStates.add(outInitState);
    outStates.add(outInitState);

    while (!pendingSTStates.isEmpty()) {

      STState currentSTState = (STState) pendingSTStates.pollFirst();

      for (String symbol : inAlphabet) {
        // The "symbol"-successor of the current subset-tuple state
        STState succSTState = new STState(id);

        StateSet seenFSAStates = new StateSet();

        // Iterate through the components of this state from right to left
        for (int i = currentSTState.numberOfComponents() - 1; i >= 0; i--) {
          StateSet succFSAStates = in.getSuccessors(currentSTState.getComponent(i).getStateSet(), symbol);
          succFSAStates.removeAll(seenFSAStates);
          seenFSAStates.addAll(succFSAStates);
          StateSet accFSAStates = new StateSet();
          StateSet naccFSAStates = new StateSet();
          for (State state : succFSAStates) {
            if (inAccStates.contains(state)) accFSAStates.add(state);
            else naccFSAStates.add(state);
          }

          if (!accFSAStates.isEmpty()) succSTState.addLeft(succSTState.new Component(accFSAStates, -1));
          if (!naccFSAStates.isEmpty()) succSTState.addLeft(succSTState.new Component(naccFSAStates, -1));

        }

        // Does succSTState alread  y exist in the automaton?
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
          out.addState(succSTState);
          outStates.add(succSTState);
          pendingSTStates.add(succSTState);
          id++;
        }
        out.createTransition(currentSTState, succSTState, symbol);
      }
      
    }

    return out;

    

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
    // acc.add(q2);
    // out.setAcc(acc);

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

    // q0.setLabel("(({s0,s" + q1.getID() + "},-1),({s2},-1),({s3},-1))");

    // return out;
  }

}
