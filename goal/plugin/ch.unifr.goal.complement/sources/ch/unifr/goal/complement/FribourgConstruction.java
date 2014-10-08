/*============================================================================*
 * Author: Daniel Weibel <daniel.weibel@unifr.ch>
 * Last modified: 5 Oct. 2014
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
import org.svvrl.goal.core.comp.ComplementConstruction;
import org.svvrl.goal.core.aut.opt.StateReducer;
import ch.unifr.goal.complement.STState.Component;



/*----------------------------------------------------------------------------*
 * Implementation of the Fribourg construction. This same class is used by both,
 * the command line and the GUI.
 *----------------------------------------------------------------------------*/
// Object > AbstractAlgorithm > AbstractControllableAlgorithm > AbstractEditableAlgorithm > ComplementConstruction
public class FribourgConstruction extends ComplementConstruction<FSA, FSA> {

  // Holding the complement if complement() has already been executed before
  private FSA complement = null;

  // Holding the intermediate results of the construction for the step-by-step
  // execution (used by getIntermediateResult()).
  private FSA in;
  private FSA out;
  private boolean preprocessPhase;

  // Each FribourgConstruction is initalised with a FribourgOptions containing
  // values for all the options for the FribourgConstruction
  private FribourgOptions options;

  /* Constructor */
  public FribourgConstruction(FSA in, FribourgOptions options) {
    super(in);
    if (!OmegaUtil.isNBW(in)) // Applicability test needed for command line
      throw new IllegalArgumentException(Message.onlyForFSA(BuchiAcc.class));
    this.options = options;
  }

  @Override // Method of interface Algorithm
  public FribourgOptions getOptions() {
    return options;
  }

  // Return current state of the constructed automaton after a step or stage
  // during the step-by-step execution of the algorithm.
  @Override // Method of interface EditableAlgorithm
  public Editable getIntermediateResult() {
    if (preprocessPhase) return in;
    else return out;
  }

  @Override // Abstract method of ComplementConstruction
  public FSA complement() {
    if (complement != null) return complement;
    fireReferenceChangedEvent(); // Method of AbstractControllableAlgorithm
    complement = construction(getInput().clone());
    return complement;
  }


  /* The implementation of the Fribourg complementation construction */
  private FSA construction(FSA input) {

    // The input automaton to complement
    in = input;

    /* -----------------------------------------------------------------------*
     * Preprocessing phase
     *   - Translate alphabet to classical (if it is propositional)
     *   - Make automaton complete (if -c is set)
     *   - Maximise the accepting state set (if -macc is set)
     * -----------------------------------------------------------------------*/
    preprocessPhase = true;
    stage("Stage 0: Preprocessing input automaton");
    boolean somethingToPreprocess = false;
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
      step("Input automaton has propositional alphabet. Converting it to classical.");
      somethingToPreprocess = true;
    }
    // If the make complete option is set, and the input automaton is not
    // complete, make it complete.
    if (getOptions().isC()) {
      String msg = "Making input automaton complete";
      if (!isComplete(in)) {
        OmegaUtil.makeTransitionComplete(in);
        msg += ".";
      }
      else msg += ": automaton is already complete.";
      step(msg);
      somethingToPreprocess = true;
    }
    // If the -rr option is set, remove unreachable and dead states from the
    // input automaton
    if (getOptions().isRR()) {
      String msg = "Removing unreachable and dead states: ";
      int statesBefore = in.getStateSize();
      StateReducer.removeUnreachable(in);
      StateReducer.removeDead(in);
      int statesRemoved = statesBefore - in.getStateSize();
      msg += "removed " + statesRemoved + " " + (statesRemoved==1?"state":"states") + ".";
      step(msg);
      somethingToPreprocess = true;
    }
    // If the -macc option is set, maximise the accepting set, i.e. make as many
    // non-accepting states as possible accepting without changing the language.
    if (getOptions().isMacc()) {
      String msg = "Maximising accepting set: ";
      StateSet additionalAccStates = OmegaUtil.maximizeAcceptingSet(in);
      if (!additionalAccStates.isEmpty()) msg += "made {" + Util.printStateSet(additionalAccStates) + "} accepting.";
      else msg += "no states could be made accepting.";
      step(msg);
      somethingToPreprocess = true;
    }
    // If there was nothing to preprocess
    if (!somethingToPreprocess) step("Nothing to preprocess.");
    preprocessPhase = false;


    /* -----------------------------------------------------------------------*
     * Complementation phase
     *   - Stage 1: construct upper part of complement automaton
     *   - Stage 2: construct lower part of complement automaton
     * -----------------------------------------------------------------------*/
    // The input automaton (which now in any case has a classical alphabet)
    String[] inAlphabet = in.getAlphabet();
    State inInitState = in.getInitialState();
    BuchiAcc inAccStates = (BuchiAcc) in.getAcc();
    boolean inIsComplete = isComplete(in);

    // The output automaton (which always has a classical alphabet as well; if
    // the input automaton was propositional, the output automaton will be
    // converted to propositional at the end)
    out = new FSA(AlphabetType.CLASSICAL, Position.OnTransition);
    out.expandAlphabet(inAlphabet);
    BuchiAcc outAccStates = new BuchiAcc();

    // IDs of the states of the output automaton
    int id = 0;

    // The states of the output automaton whose successors we have to determine
    StateSet pendingStates = new StateSet();

    // We do the same construction twice with slight differences. Iteration 1
    // constructs the upper part, and iteration 2 the lower part of the automaton.
    for (int i = 1; i <= 2; i++) {
      // Adding the initial state
      if (i == 1) {
        stage("Stage 1: Constructing upper part of output automaton");
        STState outInitState = new STState(id++);
        outInitState.addComponent(outInitState.new Component(inInitState, -1));
        if (getOptions().isB()) outInitState.makeLabelBrackets();
        else outInitState.makeLabelNormal();
        out.addState(outInitState);
        out.setInitialState(outInitState);
        pendingStates.add(outInitState);
        step("Adding initial state.");
      }
      // In the second stage we process all the states of the upper automaton again
      else if (i == 2) {
        stage("Stage 2: Constructing lower part of output automaton");
        pendingStates = new StateSet(out.getStates());
      }
      
      // An element of pendingStates will be our state p and in this loop we are
      // going to construct for each symbol of the alphabet, the state q,
      // "symbol"-successor of p.
      while (!pendingStates.isEmpty()) {
        STState p = (STState) pendingStates.pollFirst();
        for (String symbol : inAlphabet) {
          STState q = new STState(id);
          // Successors of components of p that have occurred in the processed components so far
          StateSet occurredStates = new StateSet();
          // Iterate through the components of p from right to left. At each
          // time we will treat the component pj, and we are going to determine
          // the 0, 1, or 2 successor components pk of pj.
          for (int j = p.numberOfComponents()-1; j >= 0; j--) {
            Component pj = p.getComponent(j);
            StateSet pjSuccs = in.getSuccessors(pj.getStateSet(), symbol);
            pjSuccs.removeAll(occurredStates);
            occurredStates.addAll(pjSuccs);
            StateSet pjAcc = new StateSet();
            StateSet pjNonAcc = new StateSet();
            for (State state : pjSuccs) {
              if (inAccStates.contains(state)) pjAcc.add(state);
              else pjNonAcc.add(state);
            }
            // The successor of pj are divided in two sets (acc/non-acc) and we
            // are now going to treat first the acc then the non-acc set. Each
            // of them can give rise to a component qk of the successor state q.
            // For a qk, we have to determine its colour and its state set.
            final int ACC = 0, NONACC = ACC+1;
            for (int t = ACC; t <= NONACC; t++) {
              int qkColor = 999;
              StateSet qkStates;
              if (t == ACC) qkStates = pjAcc;
              else qkStates = pjNonAcc;
              // Switches that are set if qk gets one of these roles:
              boolean qkM2 = false;         // Is qk the M2 of q?
              boolean qkM2Exclude = false;  // Is qk the colour 1 comp. that cannot be the new M2 of q? (If the M2 of p disappeared)

              // If we are constructing the upper part of the automaton
              if (i == 1) {
                if (qkStates.isEmpty()) continue;
                qkColor = -1;
              }
              // If we are construction the lower part of the automaton AND the
              // colour 2 reduction is ON
              else if (getOptions().isM2()) {
                // If p contains only colour 0 or -1 components. p has no M2.
                if (p.hasOnlyColor0OrMinus1()) {
                  if (qkStates.isEmpty()) continue;
                  switch (t) {
                    case ACC:
                      if (!q.hasColor2()) {
                        qkColor = 2;
                        qkM2 = true;
                      }
                      else qkColor = 1;
                      break;
                    case NONACC: qkColor = 0;
                  }
                }
                // If p contains colour 1 or 2 comps. p has a colour 1 or 2 M2.
                else {
                  // If the current component of p, pj, IS the M2 of p
                  if (pj == p.getM2()) {
                    switch (t) {
                      case ACC:
                        // M2 has no acceptin successors
                        if (qkStates.isEmpty()) continue;
                        // M2 has accepting successors
                        qkColor = 2;
                        qkM2 = true;
                        break;
                      case NONACC:
                        // M2 has accepting AND non-accepting successors
                        if (!qkStates.isEmpty() && !pjAcc.isEmpty())
                          qkColor = 2; // Will be merged with the acc child of pj
                        // M2 has only non-accepting successors
                        else if (!qkStates.isEmpty() && pjAcc.isEmpty()) {
                          qkColor = 2;
                          qkM2 = true;
                        }
                        // M2 has only accepting successors
                        else if (qkStates.isEmpty() && !pjAcc.isEmpty())
                          continue;
                        // pj, the M2 of p, has no successor. It disappears. We
                        // will elect one of the comps. of p as p's M2, once p
                        // is fully constructed.
                        else if (qkStates.isEmpty() && pjAcc.isEmpty()) {
                          // Mark the position in q where the child of M2(p)
                          // (and thus M2(q)) WOULD BE if M2(p) would have a child
                          q.markM2Disappearance();
                          continue;
                        }
                    }
                  }
                  // If the current component of p, pj, IS NOT the M2 of p.
                  else {
                    if (qkStates.isEmpty()) continue;
                    switch (pj.getColor()) {
                      case 1:
                        qkColor = 1;
                        break;
                      case 0:
                        if (t == ACC) {
                          qkColor = 1;
                          // Our condition is that if M2(p) disappears, then M2(q)
                          // will be the first () to the left of where the child
                          // of M2(p) WOULD BE, that is NOT A CHILD of the imme-
                          // diate {}-left-neighbour of M2(p). The parent of qk is
                          // pj. That is, if pj IS the left neighbour of M2(p), qk
                          // has colour 1, and M2(p) has indeed disappeared, then
                          // qk is exactly that component of q that has to be ex-
                          // cluded from the election of M2(q) later.
                          // Cyclic: e.g. ([],..,{}), {} is to the left of []
                          if (p.isM2Disappeared() && pj == p.getLeftNeighborOfM2())
                            qkM2Exclude = true;
                        }
                        else qkColor = 0;
                    }
                  }
                }
              }
              // If we are constructing the lower part of the automaton and the
              // colour 2 reduction optimisation is OFF
              else {
                if (qkStates.isEmpty()) continue;
                if (!p.hasColor2()) {
                  switch (pj.getColor()) {
                    case -1: case 0:
                      if (t == ACC) qkColor = 2;
                      else qkColor = 0;
                      break;
                    case 1:
                      qkColor = 2;
                  }
                }
                else {
                  switch (pj.getColor()) {
                    case 0:
                      if (t == ACC) qkColor = 1;
                      else qkColor = 0;
                      break;
                    case 1:
                      qkColor = 1;
                      break;
                    case 2:
                      qkColor = 2;
                  }
                }
              }
              // At this point, we have determined the (non-empty) state set and colour of qk
              STState.Component qk = q.new Component(qkStates, qkColor);
              if (getOptions().isM()) {
                // Add qk to left edge of q, and merge it if possible with the
                // old leftmost component of q
                q.addComponentWithMerging(qk);
                if (getOptions().isM2()) { // If qk has a special role, set it
                  if (qkM2) q.setM2(qk);                     // qk has colour 2, cannot be merged
                  if (qkM2Exclude) q.setM2ExcludeLeftmost(); // qk has colour 1, can be merged
                }
              }
              else q.addComponent(qk);   
            } // End of iterating through the two successor sets of pj
          } // End of iterating through components of state p

          // At this point, we finished constructing q.
          // If q is empty, it means that p has no outgoing transition for the
          // current symbol. We will take care of incomplete states at the end
          // of stage 2. The continue jumps to the next symbol of the alphabet.
          if (q.numberOfComponents() == 0) continue;

          if (getOptions().isM2()) {
            // If M2(p) disappeared, then q has not yet an M2(q), and we have to
            // elect one. Remember that we have postponed this decision to the
            // time when q will have been fully constructed. Of course, q needs
            // an M2 only, if it contains at least one colour 1 component.
            if (q.isM2Disappeared() && !q.hasOnlyColor0OrMinus1()) {
              int start = q.getDisappearedM2Position();
              int size = q.numberOfComponents();
              // Iterate through the comps of q, starting from the index where
              // the child of M2(p) would be, to the left, cyclically. E.g. <- 2 <- 1 <- 0 | 4 <- 3
              for (int k = 0; k < size; k++) {
                Component candidate = q.getComponent(Util.mod((start-k),size));
                // The first comp. with colour 1 and that is not the marked child of
                // the {}-left-neighbour of the disappeared M2(p) becomes M2(q).
                if (candidate.getColor() == 1 && candidate != q.getM2Exclude()) {
                  q.setM2(candidate);
                  break;
                }
              }
              // If no M2(q) has been elected above, then q contains only exactly
              // one (), and this is the m2Exclude. Since it is the last () in q,
              // we can set it as M2(q).
              if (!q.hasM2()) q.setM2(q.getM2Exclude());
            }
          }
          // q is now really finished. We can create its label.
          if (getOptions().isB()) q.makeLabelBrackets();
          else q.makeLabelNormal();
          // Does q already exist in the automaton?
          boolean qAlreadyExists = false;
          for (State state : out.getStates()) {
            if (q.equals(state)) {
              q = (STState) state;
              qAlreadyExists = true;
              break;
            }
          }
          // If the state doesn't yet exist, add it to the automaton
          if (!qAlreadyExists) {
            // If the option "If input automaton is complete, apply rightmost
            // colour 2 optimisation" is set AND the input automaton is
            // complete, then apply the rightmost colour 2 optimisation, i.e.
            // do not add the state if its rightmost colour is 2.
            if (i == 2 && getOptions().isR2ifc() && inIsComplete)
               if (q.colorOfRightmostComponent() == 2) continue;
            out.addState(q);
            pendingStates.add(q);
            id++;
            // Add states containing no colour 2 to the accepting set
            if (i == 2 && !q.hasColor2()) outAccStates.add(q);
          }
          out.createTransition(p, q, symbol);
        } // End of interating through symbols of alphabet
        step("Determining successors of state " + Preference.getStatePrefix() + p.getID() + ".");
      } // End of interating through pending states
      if (i == 1) step("Construction of upper part complete.");
      if (i == 2) step("Construction of lower part complete.");
    } // End of iterating through stage 1 and stage 2

    // If the upper part of the constructed automaton is not complete (this can
    // happen if the input automaton was not complete), we have to make it
    // complete by adding a sink state. This sink state has to be accepting.
    // Note that we could also do this at the end of stage 1, but it doesn't
    // matter, because the sink state would have no influence on the construc-
    // tion of the lower part, and the operations on the lower part in stage 2
    // have no influence on the completeness of the upper part.
    if (!isUpperPartComplete(out)) {
      STState sinkState = makeUpperPartComplete(out, id);
      outAccStates.add(sinkState);
      step("Upper part of output automaton is not complete. Making it complete by adding a sink state.");
    }
    // Set the accepting states
    out.setAcc(outAccStates);
    step("Setting accepting states.");


    /* -----------------------------------------------------------------------*
     * Postprocessing phase
     *   - Remove unreachable and dead states (if -r is set)
     *   - Convert alphabet back to propositional (if it was propositional)
     * -----------------------------------------------------------------------*/
    stage("Stage 3: Postprocessing output automaton");
    boolean somethingToPostprocess = false;
    // If the -r option is set, remove unreachable and dead states
    if (getOptions().isR()) {
      String msg = "Removing unreachable and dead states: ";
      int statesBefore = out.getStateSize();
      StateReducer.removeUnreachable(out);
      StateReducer.removeDead(out);
      int statesRemoved = statesBefore - out.getStateSize();
      msg += "removed " + statesRemoved + " " + (statesRemoved==1?"state":"states") + ".";
      step(msg);
      somethingToPostprocess = true;
    }
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
      step("Converting alphabet back to propositional.");
      somethingToPostprocess = true;
    }
    if (!somethingToPostprocess) step("Nothing to postprocess.");

    stage("Fribourg Construction finished \\(^_^)/");
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
  } // End of construction()

  /* Steps and stages for the step-by-step execution. A stage has many steps. */
  private void step(String message) {
    pause(message + "\n");
    fireReferenceChangedEvent();
  }
  private void stage(String message) {
    stagePause(message + "\n");
    fireReferenceChangedEvent();
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
    if (getOptions().isC()) s += "true, ";
    else s += "false, ";
    s += "IgnoreRightColor2=";
    if (getOptions().isR2ifc()) s += "true";
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
