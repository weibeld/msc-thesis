/*============================================================================*
 * Author: Daniel Weibel <daniel.weibel@unifr.ch>
 * Last modified: 5 Oct. 2014
 *============================================================================*/
package ch.unifr.goal.complement;

import org.svvrl.goal.core.Preference;
import org.svvrl.goal.core.aut.State;
import org.svvrl.goal.core.aut.StateSet;

/*----------------------------------------------------------------------------*
 * Contains various static utility methods.
 *----------------------------------------------------------------------------*/
public class Util {

  /* Return a string of the form "q1,q2,q5,q7" based on the states in the state
   * set which is passed as argument. */
  public static String printStateSet(StateSet stateSet) {
    String statePrefix = Preference.getStatePrefix(); // q or s
    String s = "";
    for (State state : stateSet) s += statePrefix + state.getID() + ",";
    s = s.substring(0, s.length()-1); // Remove last superfluous comma
    return s;
  }

  /* Modulo function with only positive outputs. e.g. -2 mod 7 = 5. The standard
   * Java modulo implementation (%) yields also negative values, for example,
   * -2  mod 7 = -2. */
  public static int mod(int a, int b) {
    int result = a % b;
    if (result < 0) return result + b;
    else return result;
  }

}