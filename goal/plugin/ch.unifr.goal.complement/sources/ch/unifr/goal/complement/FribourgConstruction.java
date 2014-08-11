package ch.unifr.goal.complement;

import java.util.List;
import java.util.Set;
import java.util.HashSet;
import java.util.Arrays;
import java.util.Map;
import java.util.Map.Entry;
import java.util.HashMap;
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
import org.svvrl.goal.core.Properties;
import ch.unifr.goal.complement.STState.Component;

import org.svvrl.goal.core.util.PowerSet;


/*----------------------------------------------------------------------------*
 * Implementation of the Fribourg Büchi complementation construction. This same
 * class is used by both, the command line and the GUI
 * Daniel Weibel, 25.07.2014
 *----------------------------------------------------------------------------*/

// Object > AbstractAlgorithm > AbstractControllableAlgorithm > AbstractEditableAlgorithm > ComplementConstruction
public class FribourgConstruction extends ComplementConstruction<FSA, FSA> {
  // Holding the complement if complement() has already been executed before
  private FSA complement = null;

  // Each FribourgConstruction is initalised with a FribourgOptions containing
  // values for all the options for the FribourgConstruction
  private FribourgOptions options;

  /* Constructor */
  public FribourgConstruction(FSA in, FribourgOptions options) {
    super(in);
    if (!OmegaUtil.isNBW(in)) throw new IllegalArgumentException(Message.onlyForFSA(BuchiAcc.class));
    this.options = options;
  }

  @Override // Method of interface Algorithm
  public FribourgOptions getOptions() {
    return options;
  }

  @Override // Method of interface EditableAlgorithm
  public Editable getIntermediateResult() {
    return complement;
  }

  @Override // Abstract method of ComplementConstruction
  public FSA complement() {
    if (complement != null) return complement;
    fireReferenceChangedEvent(); // Method of AbstractControllableAlgorithm
    complement = construction(getInput().clone());
    return complement;
  }

  /* The implementation of the Fribourg complementation construction */
  private FSA construction(FSA in) {

    /*** Convert input automaton from PROPOSITIONAL to CLASSICAL alphabet ***/
    /* We handle input automata with propositional alphabet by converting them
     * to classical before the construction, and converting them back to
     * propositional after the construction. */
    // An automaton A with a PROPOSITIONAL alphabet has a set of atomic
    // propositions, e.g.
    //    {p, q}                        --> A.getAtomicPropositions()
    // The alphabet of A is then
    //    {p q, p ~q, ~p q, ~p ~q}      --> A.getAlphabet()
    // Converting A to an automaton with a CLASSICAL alphabet is then done by
    // mapping the elements of A's alphabet to classical symbols, e.g.
    //    Mapping = {a <- p q,  b <- p ~q,  c <- ~p q,  d <- ~p ~q}
    // The whole conversion to classical is done by
    //    AlphabetType.CLASSICAL.convertFrom(A, Mapping)
    Map<String,String> alphabetMapping = new HashMap<String,String>();
    if (in.getAlphabetType() == AlphabetType.PROPOSITIONAL) {
      String[]     p = in.getAlphabet();
      List<String> c = AlphabetType.CLASSICAL.genAlphabet(p.length);
      for (int i = 0; i < p.length; i++) alphabetMapping.put(p[i], c.get(i));
      AlphabetType.CLASSICAL.convertFrom(in, alphabetMapping);
    }

    // If the make complete option is set, and the input automaton is not
    // complete, make it complete.
    if (getOptions().isCompl() && !isComplete(in)) OmegaUtil.makeTransitionComplete(in);

    // The input automaton (which now in any case has a classical alphabet):
    // alphabet, initial state, accepting states, and wheter it is complete
    String[] inAlphabet = in.getAlphabet();
    State inInitState = in.getInitialState();
    BuchiAcc inAccStates = (BuchiAcc) in.getAcc();
    boolean inIsComplete = isComplete(in);

    // The output automaton (which always has a classical alphabet as well; if
    // the input automaton was propositional, the output automaton will be
    // converted to propositional at the end): alphabet and accepting states
    FSA out = new FSA(AlphabetType.CLASSICAL, Position.OnTransition);
    out.expandAlphabet(inAlphabet);
    BuchiAcc outAccStates = new BuchiAcc();

    // IDs of the states of the output automaton
    int id = 0;

    // The states of the output automaton whose successors we have to determine
    StateSet pendingSTStates = new StateSet();

    // We do the same construction twice with slight differences. Iteration 1
    // constructs the upper part, and iteration 2 the lower part of the automaton.
    for (int i = 1; i <= 2; i++) {
      // Adding the initial state
      if (i == 1) {
        STState outInitState = new STState(id++);
        outInitState.addComponent(outInitState.new Component(inInitState, -1));
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

              // int color = -1;
              // if (i == 1) color = -1;   // Stage 1 (upper part of automaton)
              // else if (i == 2) {        // Stage 2 (lower part of automaton)
              //   int predColor = currentComponent.getColor();
              //   if (currentSTState.containsColor2()) {
              //     if (predColor == 0 && k != 0) color = 0;
              //     else if (predColor == 2) color = 2;
              //     else color = 1;
              //   }
              //   else {
              //     if ((predColor == 0 || predColor == -1) && k != 0) color = 0;
              //     else color = 2;
              //   }
              // }

              int color = 999;
              if (i == 1)
                color = -1;
              else {
                if (!currentSTState.containsColor2()) {
                  switch (currentComponent.getColor()) {
                    case -1: case 0:
                      if (k == 0) color = 2;
                      else color = 0;
                      break;
                    case 1:
                      color = 2;
                  }
                }
                else {
                  switch (currentComponent.getColor()) {
                    case 0:
                      if (k == 0) color = 1;
                      else color = 0;
                      break;
                    case 1:
                      color = 1;
                      break;
                    case 2:
                      color = 2;
                  }
                }
              }

              STState.Component comp = succSTState.new Component(currentSet, color);
              // Add the component to the successor state. If the merging opt-
              // imisation is on, if possible, merge the component with the current
              // leftmost component, else add it as the new leftmost component.
              if (getOptions().isMerge()) succSTState.addComponentWithMerging(comp);
              else succSTState.addComponent(comp);
            }
          } // End of iterating through all the components of the current state

          // If the successor state is empty, it means that currentSTState has
          // no successor of the current symbol, i.e. it is incomplete. We will
          // take care of incomplete states (of the upper part of the automaton)
          // at the end of stage 2. 'continue' jumps to the next symbol.
          if (succSTState.numberOfComponents() == 0) continue;

          // Does succSTState already exist in the automaton?
          succSTState.makeLabel();
          boolean alreadyExists = false;
          for (State existingState : out.getStates()) {
            if (succSTState.equals(existingState)) {
              succSTState = (STState) existingState;
              alreadyExists = true;
              break;
            }
          }
          // If the state doesn't yet exist, add it to the automaton
          if (!alreadyExists) {
            // If the option "If input automaton is complete, apply rightmost
            // colour 2 optimisation" is set AND the input automaton is
            // complete, then apply the rightmost colour 2 optimisation, i.e.
            // do not add the state if its rightmost colour is 2.
            if (i == 2 && getOptions().isRight2IfCompl() && inIsComplete)
              // The 'continue' jumps to the next symbol of the alphabet
              if (succSTState.colorOfRightmostComponent() == 2) continue;
            out.addState(succSTState);
            pendingSTStates.add(succSTState);
            id++;
            // Add states containing no colour 2 to the accepting set
            if (i == 2 && !succSTState.containsColor2()) outAccStates.add(succSTState);
          }
          out.createTransition(currentSTState, succSTState, symbol);
        } // End of interating through symbols of alphabet
      } // End of interating through pending states
    } // End of iterating through stage 1 and stage 2

    // If the upper part of the constructed automaton is not complete (this can
    // happen if the input automaton was not complete), we have to make it
    // complete by adding a sink state. This sink state has to be accepting.
    // Note that we could also do this at the end of stage 1, but it doesn't
    // matter, because the sink state would have no influence on the construc-
    // tion of the lower part, and the operations on the lower part in stage 2
    // have no influence on the completeness of the upper part.
    if (!isUpperPartComplete(out)) {
    //if (!isComplete(out)) {
      STState sinkState = makeUpperPartComplete(out, id);
      outAccStates.add(sinkState);
    }

    // Set the accepting states
    out.setAcc(outAccStates);

    /*** Convert output automaton from CLASSICAL to PROPOSITIONAL alphabet ***/
    // If the input automaton had a propositional alphabet, we convert the
    // classical output automaton now back to a propositional one by using the
    // (inversion of) the same mapping as the initial conversion.
    if (!alphabetMapping.isEmpty()) {
      // We can invert the map because the values of the original map are unique
      Map<String,String> invert = new HashMap<String,String>();
      for (Map.Entry<String,String> i : alphabetMapping.entrySet())
        invert.put(i.getValue(), i.getKey());
      AlphabetType.PROPOSITIONAL.convertFrom(out, invert);
    }

    return out;


    /* Automaton API summary (see API of class Automaton or FSA) */

    // Automaton
    // FSA out = new FSA(AlphabetType.CLASSICAL, Position.OnTransition);

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


  /* Checks if the automaton passed as argument is complete, i.e. every state
   * has at least one outgoing transition for every symbol of the alphabet */
  private boolean isComplete(FSA a) {
    for (State s : a.getStates())
      if (!isStateComplete(a, s)) return false;
    return true;
  }

  /* Checks if the upper part of the constructed automaton is complete. The
   * argument may only be an output automaton of the Fribourg Construction */
  private boolean isUpperPartComplete(FSA a) {
    for (State s : a.getStates())
      if (STState.isFromUpperPart(s) && !isStateComplete(a, s)) return false;
    return true;
  }

  /* Checks if a specific state of an automaton is complete, i.e. has an
   * outgoing transition for every symbol of the automaton's alphabet */
  private boolean isStateComplete(FSA a, State s) {
    return a.getSymbolsFromState(s).size() == a.getAlphabet().length;
  }


  /* Make the upper part of the constructed automaton complete by adding a sink
   * state with all the missing transitions. Return the sink state so that it
   * can be added to the accepting set of the automaton */
  private STState makeUpperPartComplete(FSA a, int id) {
    STState sink = new STState(id);
    sink.makeSinkLabel();
    a.addState(sink);
    Set<String> alphabet = new HashSet<String>(Arrays.asList(a.getAlphabet()));
    for (State state : a.getStates()) {
      if (!STState.isFromUpperPart(state)) continue;
      Set<String> should = new HashSet<String>(alphabet);
      Set<String> is = new HashSet<String>(a.getSymbolsFromState(state));
      should.removeAll(is);
      for (String symbol : should) a.createTransition(state, sink, symbol);
    }
    for (String symbol : alphabet) a.createTransition(sink, sink, symbol);
    return sink;
  }


  /* -------------------------------------------------------------------------*
   * FOR TESTING
   * -------------------------------------------------------------------------*/
  private FSA constructionTest1(FSA in) {
    FSA out = new FSA(AlphabetType.CLASSICAL, Position.OnTransition);
    FSAState q0 = out.createState();
    out.setInitialState(q0);
    BuchiAcc acc = new BuchiAcc();
    acc.add(q0);
    out.setAcc(acc);
    out.expandAlphabet(in.getAlphabet());
    String s = "Options: MakeComplete=";
    if (getOptions().isCompl()) s += "true, ";
    else s += "false, ";
    s += "IgnoreRightColor2=";
    if (getOptions().isRight2IfCompl()) s += "true";
    else s += "false";
    q0.setLabel(s);
    return out;
  }

  private FSA constructionTest2(FSA in) {
    FSA out = new FSA(AlphabetType.CLASSICAL, Position.OnTransition);
    FSAState q0 = out.createState();
    out.setInitialState(q0);
    BuchiAcc acc = new BuchiAcc();
    acc.add(q0);
    out.setAcc(acc);
    //out.expandAlphabet(in.getAlphabet());
    // String s = "completeLabel([p,q],[]': ";
    // String[] a = {"p","q"};
    // String[] b = {};
    // Set<String> props = AlphabetType.PROPOSITIONAL.completeLabels(a,b);
    String s = "Alphabet of {";
    String[] props = in.getAtomicPropositions();
    for (String p : props) s += p + ",";
    s += "}: ";
    //String[] alph = AlphabetType.PROPOSITIONAL.genAlphabet(props);
    String[] alph = in.getAlphabet();
    for (String a : alph) s += a + ", ";
    q0.setLabel(s);
    return out;
  }

  private FSA constructionTest3(FSA in) {
    Map<String,String> alphabetMapping = new HashMap<String,String>();
    String[]     p = in.getAlphabet();
    List<String> c = AlphabetType.CLASSICAL.genAlphabet(p.length);
    for (int i = 0; i < p.length; i++) alphabetMapping.put(p[i], c.get(i));
    AlphabetType.CLASSICAL.convertFrom(in, alphabetMapping);
    // Map<String,String> invert = new HashMap<String,String>();
    // for (Map.Entry<String,String> i : alphabetMapping.entrySet())
    //   invert.put(i.getValue(), i.getKey());
    // AlphabetType.PROPOSITIONAL.convertFrom(in, invert);    
    return in;
  }

}
