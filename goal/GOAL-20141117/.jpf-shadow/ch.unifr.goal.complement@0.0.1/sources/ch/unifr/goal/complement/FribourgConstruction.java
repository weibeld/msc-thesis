/*============================================================================*
 * Author: Daniel Weibel <daniel.weibel@unifr.ch>
 * Last modified: 14 Oct. 2014
 *============================================================================*/
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
import org.svvrl.goal.core.Properties;
import org.svvrl.goal.core.Preference;
import org.svvrl.goal.core.aut.BuchiAcc;
import org.svvrl.goal.core.aut.OmegaUtil;
import org.svvrl.goal.core.aut.State;
import org.svvrl.goal.core.aut.StateSet;
import org.svvrl.goal.core.aut.fsa.FSA;
import org.svvrl.goal.core.aut.AlphabetType;
import org.svvrl.goal.core.aut.Position;
import org.svvrl.goal.core.aut.fsa.FSAState;
import org.svvrl.goal.core.comp.AbstractComplementConstruction;
import org.svvrl.goal.core.aut.opt.StateReducer;
import ch.unifr.goal.complement.STState.Component;


/*----------------------------------------------------------------------------*
 * Implementation of the Fribourg construction. This same class is used by both,
 * the command line and the GUI.
 *----------------------------------------------------------------------------*/
// Object > AbstractAlgorithm > AbstractControllableAlgorithm > AbstractEditableAlgorithm > AbstractComplementConstruction
// AbsractComplementConstruction implements interface ComplementConstruction
public class FribourgConstruction extends AbstractComplementConstruction<FSA, FSA> {

  // Result of the complementation construction
  private FSA complement = null;

  // Intermediate results of the construction for the step-by-step execution
  private FSA in;
  private FSA out;
  private boolean preprocessing;

  // The options that have been set for this construction
  private FribourgOptions options;

  /* Constructor used by the complementation command */
  public FribourgConstruction(FSA in, FribourgOptions options) {
    super(in);
    if (!OmegaUtil.isNBW(in)) // Applicability test needed for command line
      throw new IllegalArgumentException(Message.onlyForFSA(BuchiAcc.class));
    this.options = options;
  }
  /* Constructor used by the containment and equivalence commands (if the
   * Fribourg construction is the default complementation construction) */
  public FribourgConstruction(FSA in) {
    this(in, new FribourgOptions()); // FribourgOptions with default settings
  }

  @Override // Method of interface Algorithm
  public FribourgOptions getOptions() {
    return options;
  }

  /* Return current state of the constructed automaton after a step or stage
   * during the step-by-step execution of the algorithm. */
  @Override // Method of interface EditableAlgorithm
  public Editable getIntermediateResult() {
    if (preprocessing) return in;
    else return out;
  }

  /* Entry point for starting the complementation */
  @Override // Inteface method of ComplementConstruction
  public FSA complement() {
    if (complement != null) return complement;
    fireReferenceChangedEvent(); // Method of AbstractControllableAlgorithm
    complement = construction(getInput().clone());
    return complement;
  }

  /* Implementation of the Fribourg construction (until end of file) */
  private FSA construction(FSA input) {

    // The input automaton to the construction
    in = input;

    // The output automaton of the construction
    out = new FSA(AlphabetType.CLASSICAL, Position.OnTransition);


    /*========================================================================*
     * PREPROCESSING INPUT AUTOMATON
     *   - Translate alphabet to classical (if it is propositional)
     *   - Make automaton complete (if -c is set)
     *   - Maximise the accepting state set (if -macc is set)
     *========================================================================*/
    preprocessing = true;
    stage("Stage 0: Preprocessing input automaton");
    boolean somethingToPreprocess = false;

    // Convert alphabet to classical
    // An automaton A with propositional alphabet has atomic propositions, e.g:
    //    {p, q}                    --> A.getAtomicPropositions()
    // The alphabet of A is then:
    //    {p q, p ~q, ~p q, ~p ~q}  --> A.getAlphabet()
    // Conversion of this alphabet to classical is done by a mapping like:
    //    Mapping = {a <- p q,  b <- p ~q,  c <- ~p q,  d <- ~p ~q}
    // The conversion of A's alphabet is done by:
    //    --> AlphabetType.CLASSICAL.convertFrom(A, Mapping)
    Map<String,String> alphabetMapping = new HashMap<String,String>();
    if (in.getAlphabetType() == AlphabetType.PROPOSITIONAL) {
      String[]     p = in.getAlphabet();
      List<String> c = AlphabetType.CLASSICAL.genAlphabet(p.length);
      for (int i = 0; i < p.length; i++) alphabetMapping.put(p[i], c.get(i));
      AlphabetType.CLASSICAL.convertFrom(in, alphabetMapping);
      step("Converting alphabet of input automaton from propositional to classical.");
      somethingToPreprocess = true;
    }

    // Remove unreachable and dead states from the input automaton
    if (options.isRR()) {
      int before = in.getStateSize();
      StateReducer.removeUnreachable(in);
      StateReducer.removeDead(in);
      int removed = before - in.getStateSize();
      step("Removing unreachable and dead states: removed " + removed + " state" + (removed==1?"":"s") + ".");
      somethingToPreprocess = true;
    }

    // Make input automaton complete
    if (options.isC()) {
      if (!isComplete(in)) {
        OmegaUtil.makeTransitionComplete(in);
        step("Making input automaton complete (-c option).");
      }
      else step("Input automaton is already complete (-c option).");
      somethingToPreprocess = true;
    }

    // Inform user whether the -r2ifc option has an effect or not
    if (options.isR2ifc()) {
      if (isComplete(in)) step("Input automaton is complete. Can apply -r2ifc optimisation.");
      else step("Input automaton is not complete. Cannot apply -r2ifc optimisation.");
    }

    // Maximise the accepting set of the input automaton, that is, making as
    // many states as possible accepting without changing the language
    if (options.isMacc()) {
      String msg = "Maximising accepting set: ";
      StateSet added = OmegaUtil.maximizeAcceptingSet(in);
      if (!added.isEmpty()) msg += "made {" + Util.printStates(added) + "} accepting.";
      else msg += "no states could be made accepting.";
      step(msg);
      somethingToPreprocess = true;
    }

    // If no preprocessing was done
    if (!somethingToPreprocess) step("Nothing to preprocess.");
    preprocessing = false;

    // Information possibly affected by preprocessing
    out.expandAlphabet(in.getAlphabet());  // Set alphabet of output automaton
    boolean isCompleteIn = isComplete(in); // Is the input automaton complete?

    /*========================================================================*
     * COMPLEMENTATION
     *   - Stage 1: construct upper part of complement automaton
     *   - Stage 2: construct lower part of complement automaton
     *========================================================================*/
    BuchiAcc accSetOut = new BuchiAcc();     // Accepting set of the complement
    StateSet pendingStates = new StateSet(); // Queue of the states to process
    int id = 0;                              // State IDs of the complement

    // The two stages of the construction, 1: upper part, 2: lower part
    for (int i = 1; i <= 2; i++) {
      // Adding the initial state
      if (i == 1) {
        stage("Stage 1: Constructing upper part of output automaton");
        STState outInitState = new STState(id++);
        outInitState.addComponent(outInitState.new Component(in.getInitialState(), -1));
        outInitState.makeLabel(options.isB());
        out.addState(outInitState);
        out.setInitialState(outInitState);
        pendingStates.add(outInitState);
        step("Adding initial state.");
      }
      // Stage 2: process all states in the intermediate automaton again
      else if (i == 2) {
        stage("Stage 2: Constructing lower part of output automaton");
        pendingStates = new StateSet(out.getStates());
      }
      
      // For each state p for which we have to determine its successor states
      while (!pendingStates.isEmpty()) {
        STState p = (STState) pendingStates.pollFirst();
        step("Determining successors of state " + Util.printState(p) + ".");
        // A successor state q of p for every symbol of the alphabet
        for (String symbol : in.getAlphabet()) {
          STState q = new STState(id);
          StateSet occurredStates = new StateSet();
          boolean createdColor2 = false; // Used for the -m2 option only
          // Iterate through the components of p from right to left. For each
          // component we determine the acc and nonacc successor sets and their
          // colors, and add them to q (if not empty).
          for (int j = p.numberOfComponents()-1; j >= 0; j--) {
            Component pj = p.getComponent(j);
            StateSet pjSuccs = in.getSuccessors(pj.stateSet(), symbol);
            pjSuccs.removeAll(occurredStates);
            occurredStates.addAll(pjSuccs);
            StateSet acc = new StateSet();
            StateSet nonacc = new StateSet();
            for (State state : pjSuccs) {
              if (isAccepting(in, state)) acc.add(state);
              else nonacc.add(state);
            }
            // Without the -m2 option
            if (!options.isM2()) {
              if (p.hasColor2()) {
                if (pj.color() == 0) {
                  q.addComponent(q.new Component(acc,    1));
                  q.addComponent(q.new Component(nonacc, 0));
                }
                else if (pj.color() == 1) {
                  q.addComponent(q.new Component(acc,    1));
                  q.addComponent(q.new Component(nonacc, 1));
                }
                else if (pj.color() == 2) {
                  q.addComponent(q.new Component(acc,    2));
                  q.addComponent(q.new Component(nonacc, 2));
                }
              }
              else {
                if (pj.color() == -1 && i == 1) { // When building upper part
                  q.addComponent(q.new Component(acc,    -1));
                  q.addComponent(q.new Component(nonacc, -1));
                }
                else if (pj.color() == 0 || pj.color() == -1) {
                  q.addComponent(q.new Component(acc,    2));
                  q.addComponent(q.new Component(nonacc, 0));
                }
                else if (pj.color() == 1) {
                  q.addComponent(q.new Component(acc,    2));
                  q.addComponent(q.new Component(nonacc, 2));
                }
              }
            }
            // With the -m2 option
            else {
              // p has exactly one 2-coloured component
              if (p.hasColor2()) {
                if (pj.color() == 0) {
                  q.addComponent(q.new Component(acc,    1));
                  q.addComponent(q.new Component(nonacc, 0));
                }
                else if (pj.color() == 1) {
                  q.addComponent(q.new Component(acc,    1));
                  q.addComponent(q.new Component(nonacc, 1));
                }
                else if (pj.color() == 2) {
                  if (acc.isEmpty() && nonacc.isEmpty())
                    q.addComponent(q.new EmptyColor2Component());
                  else {
                    q.addComponent(q.new Component(acc,    2));
                    q.addComponent(q.new Component(nonacc, 2));
                  }
                }
              }
              // p contains only -1, 0, or 1-coloured components. Only one
              // of them may have 2-coloured successors.
              else {
                if (pj.color() == -1 && i == 1) { // When building upper part
                  q.addComponent(q.new Component(acc,    -1));
                  q.addComponent(q.new Component(nonacc, -1));
                }
                else if (pj.color() == 0 || pj.color() == -1) {
                  if (!createdColor2) {
                    q.addComponent(q.new Component(acc,    2));
                    q.addComponent(q.new Component(nonacc, 0));
                    if (!acc.isEmpty()) createdColor2 = true;
                  }
                  else {
                    q.addComponent(q.new Component(acc,    1));
                    q.addComponent(q.new Component(nonacc, 0));
                  }
                }
                else if (pj.color() == 1) {
                  if (!createdColor2) {
                    q.addComponent(q.new Component(acc,    2));
                    q.addComponent(q.new Component(nonacc, 2));
                    if (!acc.isEmpty() || !nonacc.isEmpty()) createdColor2 = true;
                  }
                  else {
                    q.addComponent(q.new Component(acc,    1));
                    q.addComponent(q.new Component(nonacc, 1));
                  }
                }
              }
            }
          } // End of iterating through components of state p

          // Apply merging optimisation
          if (options.isM()) q.mergeComponents();

          // If the previous 2-coloured component has been emptied in q
          if (options.isM2() && q.hasEmptyColor2()) {
            // If q has no 0-coloured components, it can be discarded
            if (!q.hasColor0()) continue;
            // If q has 1-coloured components, one of them is made 2-coloured
            if (q.hasColor1()) {
              q.getColor1ToMakeColor2().setColor(2);
              q.setAccFlag();
              q.mergeComponents();
            }
            q.removeEmptyColor2();
          }

          // If the -r2ifc option is on, discard state with rightmost colour 2
          if (options.isR2ifc() && isCompleteIn && q.isRightmostColor2() && !q.hasAccFlag())
            continue;

          // If the state is empty, discard it
          if (q.numberOfComponents() == 0) continue;

          // Create and add the label for q
          q.makeLabel(options.isB());

          // Does a state similar to q already exist?
          STState similar = getSimilar(out, q);
          // If not, add q to the automaton
          if (similar == null) {
            out.addState(q);
            pendingStates.add(q);
            if ((i == 2 && !q.hasColor2()) || q.hasAccFlag()) accSetOut.add(q);
            out.createTransition(p, q, symbol);
            id++;
          }
          // If a similar state exists, just create a transition to this state
          else out.createTransition(p, similar, symbol);
        } // End of interating through symbols of alphabet

      } // End of interating through pending states

      if (i == 1) step("Construction of upper part complete.");
      if (i == 2) step("Construction of lower part complete.");
    } // End of iterating through stage 1 and stage 2

    // If the upper part is not complete, which can happen if the input auto-
    // maton is not complete, make it complete by adding a sink state.
    if (!isUpperPartComplete(out)) {
      STState sinkState = makeUpperPartComplete(out, id);
      accSetOut.add(sinkState);
      step("Making upper part of automaton complete by adding a sink state.");
    }

    // Set the accepting states
    out.setAcc(accSetOut);
    step("Setting accepting states.");


    /*========================================================================*
     * POSTPROCESSING OUTPUT AUTOMATON
     *   - Remove unreachable and dead states (if -r is set)
     *   - Convert alphabet back to propositional (if it was propositional)
     *========================================================================*/
    stage("Stage 3: Postprocessing output automaton");
    boolean somethingToPostprocess = false;

    // If the -r option is set, remove unreachable and dead states
    if (options.isR()) {
      int before = out.getStateSize();
      StateReducer.removeUnreachable(out);
      StateReducer.removeDead(out);
      int removed = before - out.getStateSize();
      step("Removing unreachable and dead states: removed " + removed + " state" + (removed==1?"":"s") + ".");
      somethingToPostprocess = true;
    }

    // Convert alphabet back to propositional
    if (!alphabetMapping.isEmpty()) {
      // We can invert the map because the values of the original map are unique
      Map<String,String> invert = new HashMap<String,String>();
      for (Map.Entry<String,String> i : alphabetMapping.entrySet())
        invert.put(i.getValue(), i.getKey());
      AlphabetType.PROPOSITIONAL.convertFrom(out, invert);
      step("Converting alphabet back to propositional.");
      somethingToPostprocess = true;
    }

    // If no postprocessing was done
    if (!somethingToPostprocess) step("Nothing to postprocess.");

    // Finished!
    stage("Fribourg Construction finished \\(^_^)/");
    return out;
  } // End of construction


  /*==========================================================================*
   * Private helper methods
   *==========================================================================*/

  /* Steps and stages for the step-by-step execution. A stage has many steps. */
  private void step(String message) {
    pause(message + "\n");
    fireReferenceChangedEvent();
  }
  private void stage(String message) {
    stagePause(message + "\n");
    fireReferenceChangedEvent();
  }

  /* Returns the state in automaton a that is identical to the state s, or null
   * if a contains no state identical to s. */
  private STState getSimilar(FSA a, STState s) {
    for (State existing : a.getStates())
      if (s.equals(existing)) return (STState) existing;
    return null;
  }

  /* Checks if a given state is an accepting state of a given automaton */
  private boolean isAccepting(FSA a, State s) {
    BuchiAcc accStates = (BuchiAcc) a.getAcc();
    return accStates.contains(s);
  }

  /* Checks if the automaton passed as argument is complete, i.e. every state
   * has at least one outgoing transition for every symbol of the alphabet */
  private boolean isComplete(FSA a) {
    for (State s : a.getStates())
      if (!isStateComplete(a, s)) return false;
    return true;
  }

  /* Checks if the upper part of the constructed automaton is complete. */
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
   * state. Return the added sink state. */
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
