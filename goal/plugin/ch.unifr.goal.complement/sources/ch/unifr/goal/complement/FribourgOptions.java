package ch.unifr.goal.complement;

import org.svvrl.goal.core.Properties;
import org.svvrl.goal.core.Preference;

/*----------------------------------------------------------------------------*
 * A FribourgOptions object contains the options for a FribourgConstruction as
 * key=value pairs. For creating a FribourgConstruction, one first creates a
 * FribourgOptions and passes it to the FribourgConstruction constructor.
 * ----------
 * This class has a second purpose, which is providing an interface to the
 * GOAL preference file. For every option that exists in a FribourgOptions,
 * there exist two entries in the preference file:
 * 1) <option_key>=<PREFERENCE_VALUE>: used to initialise the checkboxes in the
 *    options dialog in the GUI. Modified by clicking on 'Save as Default'.
 * 2) <option_key>=<DEFAULT_VALUE>: used when the user clicks on 'Reset' in
 *    'Preferences... > Complementation > Fribourg Construction' to reset the
 *    preference values to the default values.
 * Both, default and preference values, are persistent across runs of GOAL.
 * Daniel Weibel, 31.07.2014
 *----------------------------------------------------------------------------*/

// java.util.Properties > org.svvrl.goal.core.Properties > FribourgOptions
public class FribourgOptions extends Properties {
  
  // Option keys (same keys used in FribourgOptions and in Preference file)
  private static final String complKey;
  private static final String right2IfComplKey;
  private static final String mergeKey;
  private static final String reduce2Key;

  // Option default values that are saved in the Preference file
  private static final boolean complDefault         = false;
  private static final boolean right2IfComplDefault = true;
  private static final boolean mergeDefault         = true;
  private static final boolean reduce2Default       = true;

  // Will be executed when the class is first accessed. This is actually only
  // necessary to set the default values in the preference file the very first
  // time the plugin is run. In subsequent runs, the default values would still
  // be in the Preference file, and setting them would not be necessary.
  static {
    // Initialise keys (has to be done in this static block)
    complKey         = "compl";
    right2IfComplKey = "right2IfCompl";
    mergeKey         = "merge";
    reduce2Key       = "reduce2";
    // Set the default values
    Preference.setDefault(complKey,         complDefault);
    Preference.setDefault(right2IfComplKey, right2IfComplDefault);
    Preference.setDefault(mergeKey,         mergeDefault);
    Preference.setDefault(reduce2Key,       reduce2Default);
  }

  /* Create a FribourgOptions object with the default values for all options.
   * The idea is that after creating a FribourgOptions, the desired options are
   * set with the public setter methods */
  public FribourgOptions() {
    setProperty(complKey,         complDefault);
    setProperty(right2IfComplKey, right2IfComplDefault);
    setProperty(mergeKey,         mergeDefault);
    setProperty(reduce2Key,       reduce2Default);
  }

  /* Option getter methods */
  public boolean isCompl() {
    return getPropertyAsBoolean(complKey);
  }
  public boolean isRight2IfCompl() {
    return getPropertyAsBoolean(right2IfComplKey);
  }
  public boolean isMerge() {
    return getPropertyAsBoolean(mergeKey);
  }
  public boolean isReduce2() {
    return getPropertyAsBoolean(reduce2Key);
  }

  /* Option setter methods */
  public void setCompl(boolean value) {
    setProperty(complKey, value);
  }
  public void setRight2IfCompl(boolean value) {
    setProperty(right2IfComplKey, value);
  }
  public void setMerge(boolean value) {
    setProperty(mergeKey, value);
  }
  public void setReduce2(boolean value) {
    setProperty(reduce2Key, value);
  }


  /* Interface (read-only) to the Preference file. Each option has a persistent
   * default and preference value. */

  // The preference values are used to set the checkboxes in the options dialog
  // in the GUI. If the user clicks on 'Save as Default', the current selection
  // of options will be set as the new preference values.
  public static  boolean getComplPref() {
    return Preference.getPreferenceAsBoolean(complKey);
  }
  public static boolean getRight2IfComplPref() {
    return Preference.getPreferenceAsBoolean(right2IfComplKey);
  }
  public static boolean getMergePref() {
    return Preference.getPreferenceAsBoolean(mergeKey);
  }
  public static boolean getReduce2Pref() {
    return Preference.getPreferenceAsBoolean(reduce2Key);
  }

  // The default values are used when the user clicks on 'Reset' in the
  // 'Preferences... > Complementation  > Fribourg Construction' dialog to
  // reset the options to their default values (currently not implemented). The
  // default values are never changed (unless in the source of this file).
  public static boolean getComplDefault() {
    return Preference.getDefaultAsBoolean(complKey);
  }
  public static boolean getRight2IfComplDefault() {
    return Preference.getDefaultAsBoolean(right2IfComplKey);
  }
  public static boolean getMergeDefault() {
    return Preference.getDefaultAsBoolean(mergeKey);
  }
  public static boolean getReduce2Default() {
    return Preference.getDefaultAsBoolean(reduce2Key);
  }
  
}
