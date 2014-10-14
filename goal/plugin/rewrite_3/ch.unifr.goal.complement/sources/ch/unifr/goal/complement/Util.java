/*============================================================================*
 * Author: Daniel Weibel <daniel.weibel@unifr.ch>
 * Last modified: 14 Oct. 2014
 *============================================================================*/
package ch.unifr.goal.complement;

import java.util.List;
import java.util.LinkedList;
import org.svvrl.goal.core.Preference;
import org.svvrl.goal.core.aut.State;
import org.svvrl.goal.core.aut.StateSet;

/*----------------------------------------------------------------------------*
 * Various static utility methods.
 *----------------------------------------------------------------------------*/
public class Util {

  // Print a state set in the form "q1,q2,q5,q7"
  public static String printStates(StateSet stateSet) {
    String s = "";
    for (State state : stateSet) s += printState(state) + ",";
    s = s.substring(0, s.length()-1); // Remove last superfluous comma
    return s;
  }
  public static String printState(State state) {
    return Preference.getStatePrefix() + state.getID();
  }

  // Modulo with only positive values: -2 mod 7 = 5. Because in the standard
  // Java modulo implementation, -2  mod 7 = -2.
  public static int mod(int a, int b) {
    int result = a % b;
    if (result < 0) return result + b;
    else return result;
  }

  // Prolog-like list head. If list empty, then Error.
  public static <T> T head(List<T> list) {
    return list.get(0);
  }
  // Prolog-like list tail. If list size 1, return empty list. If list empty, then Error.
  public static <T> List<T> tail(List<T> list) {
    return list.subList(1,list.size()); // from: inclusive, to: exclusive
  }

  // Prolog-like list inversion ([1 2 3 4] -> [4 3 2 1])
  public static <T> LinkedList<T> invertList(List<T> list) {
    if (list.isEmpty()) return new LinkedList<T>();
    LinkedList<T> invertedList = invertList(tail(list));
    invertedList.add(head(list));
    return invertedList;
  }

}