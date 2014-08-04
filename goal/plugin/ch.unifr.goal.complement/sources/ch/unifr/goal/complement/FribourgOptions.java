package ch.unifr.goal.complement;

/* Daniel Weibel, 31.07.2014 */

import org.svvrl.goal.core.Properties;
import org.svvrl.goal.core.Preference;

/* A FribourgOptions object contains the options for a FribourgConstruction as
 * key=value pairs. For creating a FribourgConstruction, one first creates a
 * FribourgOptions and passes it to the FribourgConstruction constructor.
 * ----------
 * Values for all these options also exist, independently from a FribourgOptions
 * object, in two different forms in a so called preference file:
 * - DEFAULT VALUES: defined in this file and never changed. Used only for the
 *   command line mode for determining the value of options that are not
 *   specified on the command line.
 * - PREFERENCE VALUES: set only when the user clicks 'Save as Default' in the
 *   options dialog when starting the construction in the GUI. Used only for
 *   initially selecting or not selecting the checkboxes in this dialog.
 * Both, default and preference values, are persistent across runs of GOAL.
 * ----------
 * java.util.Properties > org.svvrl.goal.core.Properties > FribourgOptions */
public class FribourgOptions extends Properties {
  // The keys of all the options. The same keys are used in this object and in
  // the preference file
  private static final String delRight2Key;
  private static final String makeCompleteKey;

  // The default values for all the options. Default values will never change
  // unless they are changed here in the source file
  private static final boolean makeCompleteDefault = false;
  private static final boolean delRight2Default = false;

  // Will be executed when the class is first accessed
  static {
    // Initialise the keys
    delRight2Key = "delRight2";
    makeCompleteKey = "makeComplete";
    // Set the default values in the preferences file. This is actually only
    // necessary the very first time the plugin is executed.
    Preference.setDefault(makeCompleteKey, makeCompleteDefault);
    Preference.setDefault(delRight2Key, delRight2Default);
  }

  /* Create a FribourgOptions object with the default values for all options */
  public FribourgOptions() {
    setProperty(makeCompleteKey, makeCompleteDefault);
    setProperty(delRight2Key, delRight2Default);
  }

  /* Create a FribourgOptions object with custom values for the options. If a
   * specific option is present in the argument, set it to that value, otherwise
   * set it to the default value. */
  public FribourgOptions(Properties options) {
    // Make complete at start option
    boolean makeComplete;
    if (options.containsKey(makeCompleteKey)) makeComplete = options.getPropertyAsBoolean(makeCompleteKey);
    else makeComplete = makeCompleteDefault;
    setProperty(makeCompleteKey, makeComplete);

    // Ignore states whose rightmost component has color 2 option
    boolean delRight2;
    if (options.containsKey(delRight2Key)) delRight2 = options.getPropertyAsBoolean(delRight2Key);
    else delRight2 = delRight2Default;
    if (!makeComplete && delRight2) delRight2 = false;
    setProperty(delRight2Key, delRight2);
  }

  /* Instance methods to query the value of a specific option in this object */
  public boolean isMakeComplete() {
    return getPropertyAsBoolean(makeCompleteKey);
  }
  public boolean isDelRight2() {
    return getPropertyAsBoolean(delRight2Key);
  }

  /* Static getter methods */
  // Keys of options
  public static String getMakeCompleteKey() {
    return makeCompleteKey;
  }
  public static String getDelRight2Key() {
    return delRight2Key;
  }
  // Default values of options in the preference file. Is set in the static
  // block of this class.
  public static boolean getMakeCompleteDefault() {
    return makeCompleteDefault;
  }
  public static boolean getDelRight2Default() {
    return delRight2Default;
  }
  // Preference values of options in the preference file. Can only be set in
  // the GUI by clicking on 'Save as Default' in the OptionsDialog.
  public static  boolean getMakeCompletePref() {
    return Preference.getPreferenceAsBoolean(makeCompleteKey);
  }
  public static boolean getDelRight2Pref() {
    return Preference.getPreferenceAsBoolean(delRight2Key);
  }
}
