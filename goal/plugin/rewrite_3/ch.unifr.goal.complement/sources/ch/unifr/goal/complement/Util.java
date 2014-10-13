/*============================================================================*
 * Author: Daniel Weibel <daniel.weibel@unifr.ch>
 * Last modified: 5 Oct. 2014
 *============================================================================*/
package ch.unifr.goal.complement;

import java.util.List;
import java.util.LinkedList;
import org.svvrl.goal.core.Preference;
import org.svvrl.goal.core.aut.State;
import org.svvrl.goal.core.aut.StateSet;

/*----------------------------------------------------------------------------*
 * Contains various static utility methods.
 *----------------------------------------------------------------------------*/
public class Util {

  /* Return a string of the form "q1,q2,q5,q7" based on the states in the state
   * set which is passed as argument. */
  public static String printStates(StateSet stateSet) {
    String s = "";
    for (State state : stateSet) s += printState(state) + ",";
    s = s.substring(0, s.length()-1); // Remove last superfluous comma
    return s;
  }
  public static String printState(State state) {
    return Preference.getStatePrefix() + state.getID();
  }

  /* Modulo function with only positive outputs. e.g. -2 mod 7 = 5. The standard
   * Java modulo implementation (%) yields also negative values, for example,
   * -2  mod 7 = -2. */
  public static int mod(int a, int b) {
    int result = a % b;
    if (result < 0) return result + b;
    else return result;
  }

  /* Prolog-like head and tail of list. */
  // Return head. If list empty, then Error.
  public static <T> T head(List<T> list) {
    return list.get(0);
  }
  // Return tail. If list size 1, return empty list. If list empty, then Error.
  public static <T> List<T> tail(List<T> list) {
    return list.subList(1,list.size()); // from: inclusive, to: exclusive
  }

  /* Prolog-like list inversion ([1 2 3 4] -> [4 3 2 1]) */
  public static <T> LinkedList<T> invertList(List<T> list) {
    if (list.isEmpty()) return new LinkedList<T>();
    LinkedList<T> invertedList = invertList(tail(list));
    invertedList.add(head(list));
    return invertedList;
  }

}