package ch.unifr.goal.complement;

import org.svvrl.goal.core.Properties;
import java.util.Enumeration;


/* Dictionary > Hashtable > java.util.Properties > org.svvrl.goal.core.Properties >
 * FribourgOptions */
public class FribourgOptions extends Properties {

  // The keys for all the options (options are stored as key=value pairs)
  public static String makeCompleteKey = "makeComplete";
  public static String ignoreRightColor2Key = "ignoreRightColor2";

  // The default values of all the options
  private boolean makeCompleteDefault = false;
  private boolean ignoreRightColor2Default = false;

  // Construct a FribourgOptions object with the default values for all options
  public FribourgOptions() {
    setProperty(makeCompleteKey, makeCompleteDefault);
    setProperty(ignoreRightColor2Key, ignoreRightColor2Default);
  }

  /* Construct a FribourgOptions object with custom options */
  public FribourgOptions(Properties options) {
    // Make complete at start option
    boolean makeComplete = options.getPropertyAsBoolean(makeCompleteKey, makeCompleteDefault);
    setProperty(makeCompleteKey, makeComplete);
    // Ignore states whose rightmost color is 2 option
    boolean ignoreRightColor2 = options.getPropertyAsBoolean(ignoreRightColor2Key, ignoreRightColor2Default);
    if (!makeComplete && ignoreRightColor2) ignoreRightColor2 = false;
    setProperty(ignoreRightColor2Key, ignoreRightColor2);
  }


  public boolean isMakeComplete() {
    return getPropertyAsBoolean(makeCompleteKey);
  }

  public boolean isIgnoreRightColor2() {
    return getPropertyAsBoolean(ignoreRightColor2Key);
  }

  public void setMakeComplete(boolean value) {
    setProperty(makeCompleteKey, value);
  }

  public void setIgnoreRightColor2(boolean value) {
    if (value && isMakeComplete()) setProperty(ignoreRightColor2Key, true);
    else setProperty(ignoreRightColor2Key, false);
  }
}