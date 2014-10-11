/*============================================================================*
 * Author: Daniel Weibel <daniel.weibel@unifr.ch>
 * Last modified: 5 Oct. 2014
 *============================================================================*/
package ch.unifr.goal.complement;

import org.svvrl.goal.core.Properties;
import org.svvrl.goal.core.Preference;

/*----------------------------------------------------------------------------*
 * A FribourgOptions object contains the options for a FribourgConstruction as
 * key=value pairs. For creating a FribourgConstruction, one first creates a
 * FribourgOptions and passes it to the FribourgConstruction constructor.
 * ----------
 * This class has a second purpose, which is providing a static interface to the
 * GOAL preference file. For every option that exists in a FribourgOptions,
 * there exist two entries in the preference file:
 * 1) <option_key>=<PREFERENCE_VALUE>: used to initialise the checkboxes in the
 *    options dialog in the GUI. Modified by clicking on 'Save as Default'.
 * 2) <option_key>=<DEFAULT_VALUE>: used when the user clicks on 'Reset' in
 *    'Preferences... > Complementation > Fribourg Construction' to reset the
 *    preference values to the default values.
 * Both, default and preference values, are persistent across runs of GOAL.
 *----------------------------------------------------------------------------*/
// java.util.Properties > org.svvrl.goal.core.Properties > FribourgOptions
public class FribourgOptions extends Properties {
  
  // Option keys (same keys used in FribourgOptions and in Preference file)
  private static final String cKey;
  private static final String r2ifcKey;
  private static final String mKey;
  private static final String m2Key;
  private static final String bKey;
  private static final String maccKey;
  private static final String rKey;
  private static final String rrKey;

  // Option default values that are saved in the Preference file
  private static final boolean cDefault     = false;
  private static final boolean r2ifcDefault = true;
  private static final boolean mDefault     = true;
  private static final boolean m2Default    = false;
  private static final boolean bDefault     = false;
  private static final boolean maccDefault  = true;
  private static final boolean rDefault     = true;
  private static final boolean rrDefault    = false;

  // Will be executed when the class is first accessed. This is actually only
  // necessary to set the default values in the preference file the very first
  // time the plugin is run. In subsequent runs, the default values would still
  // be in the Preference file, and setting them would not be necessary.
  static {
    // Initialise keys (has to be done in this static block)
    cKey     = "c";
    r2ifcKey = "r2ifc";
    mKey     = "m";
    m2Key    = "m2";
    bKey     = "b";
    maccKey  = "macc";
    rKey     = "r";
    rrKey    = "rr";
    // Set the default values
    Preference.setDefault(cKey,     cDefault);
    Preference.setDefault(r2ifcKey, r2ifcDefault);
    Preference.setDefault(mKey,     mDefault);
    Preference.setDefault(m2Key,    m2Default);
    Preference.setDefault(bKey,     bDefault);
    Preference.setDefault(maccKey,  maccDefault);
    Preference.setDefault(rKey,     rDefault);
    Preference.setDefault(rrKey,    rrDefault);
  }

  /* Create a FribourgOptions object with the default values for all options.
   * The idea is however that after creating a FribourgOptions, the individual
   * options are customised with the public setter methods */
  public FribourgOptions() {
    setProperty(cKey,     cDefault);
    setProperty(r2ifcKey, r2ifcDefault);
    setProperty(mKey,     mDefault);
    setProperty(m2Key,    m2Default);
    setProperty(bKey,     bDefault);
    setProperty(maccKey,  maccDefault);
    setProperty(rKey,     rDefault);
    setProperty(rrKey,    rrDefault);
  }

  /* Option getter methods */
  public boolean isC() {
    return getPropertyAsBoolean(cKey);
  }
  public boolean isR2ifc() {
    return getPropertyAsBoolean(r2ifcKey);
  }
  public boolean isM() {
    return getPropertyAsBoolean(mKey);
  }
  public boolean isM2() {
    return getPropertyAsBoolean(m2Key);
  }
  public boolean isB() {
    return getPropertyAsBoolean(bKey);
  }
  public boolean isMacc() {
    return getPropertyAsBoolean(maccKey);
  }
  public boolean isR() {
    return getPropertyAsBoolean(rKey);
  }
  public boolean isRR() {
    return getPropertyAsBoolean(rrKey);
  }

  /* Option setter methods */
  public void setC(boolean value) {
    setProperty(cKey, value);
  }
  public void setR2ifc(boolean value) {
    setProperty(r2ifcKey, value);
  }
  public void setM(boolean value) {
    setProperty(mKey, value);
  }
  public void setM2(boolean value) {
    setProperty(m2Key, value);
  }
  public void setB(boolean value) {
    setProperty(bKey, value);
  }
  public void setMacc(boolean value) {
    setProperty(maccKey, value);
  }
  public void setR(boolean value) {
    setProperty(rKey, value);
  }
  public void setRR(boolean value) {
    setProperty(rrKey, value);
  }


  /* -------------------------------------------------------------------------*
   * Interface (read-only) to the Preference file. Each option has a persistent
   * default and preference value.
   * -------------------------------------------------------------------------*/
  // The preference values are used to set the checkboxes in the options dialog
  // in the GUI. If the user clicks on 'Save as Default', the current selection
  // of options will be set as the new preference values.
  public static boolean getCPref() {
    return Preference.getPreferenceAsBoolean(cKey);
  }
  public static boolean getR2ifcPref() {
    return Preference.getPreferenceAsBoolean(r2ifcKey);
  }
  public static boolean getMPref() {
    return Preference.getPreferenceAsBoolean(mKey);
  }
  public static boolean getM2Pref() {
    return Preference.getPreferenceAsBoolean(m2Key);
  }
  public static boolean getBPref() {
    return Preference.getPreferenceAsBoolean(bKey);
  }
  public static boolean getMaccPref() {
    return Preference.getPreferenceAsBoolean(maccKey);
  }
  public static boolean getRPref() {
    return Preference.getPreferenceAsBoolean(rKey);
  }
  public static boolean getRRPref() {
    return Preference.getPreferenceAsBoolean(rrKey);
  }
  // The default values are accessed when the user clicks on 'Reset' in
  // the 'Preferences... > Complementation  > Fribourg Construction' dialog to
  // reset the options to their default values. The default values are never
  // changed (except in this file).
  public static boolean getCDefault() {
    return Preference.getDefaultAsBoolean(cKey);
  }
  public static boolean getR2ifcDefault() {
    return Preference.getDefaultAsBoolean(r2ifcKey);
  }
  public static boolean getMDefault() {
    return Preference.getDefaultAsBoolean(mKey);
  }
  public static boolean getM2Default() {
    return Preference.getDefaultAsBoolean(m2Key);
  }
  public static boolean getBDefault() {
    return Preference.getDefaultAsBoolean(bKey);
  }
  public static boolean getMaccDefault() {
    return Preference.getDefaultAsBoolean(maccKey);
  }
  public static boolean getRDefault() {
    return Preference.getDefaultAsBoolean(rKey);
  }
  public static boolean getRRDefault() {
    return Preference.getDefaultAsBoolean(rrKey);
  }
  
}
