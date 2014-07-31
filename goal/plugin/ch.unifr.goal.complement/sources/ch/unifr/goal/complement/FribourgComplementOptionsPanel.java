package ch.unifr.goal.complement;

/* Daniel Weibel, 31.07.2014 */

import javax.swing.JCheckBox;
import java.awt.GridLayout;
import javax.swing.border.EmptyBorder;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import org.svvrl.goal.gui.pref.OptionsPanel;
import org.svvrl.goal.core.Properties;


/* The content of the OptionsDialog displayed before the construction starts.
 * Consists of a checkbox for every option. The state (checked/unchecked) of each
 * check box is according to the preference value of the option in the preference
 * file. These preference values can be changed by clicking on the 'Save as
 * Default' button in the OptionsDialog. The main goal of this class is to
 * return a FribourgOptions object that is used to create a FribourgConstruction.
 * JPanel > OptionsPanel */
public class FribourgComplementOptionsPanel extends OptionsPanel<FribourgOptions> {
  // A checkbox for every option
  final JCheckBox makeCompleteCheckBox;
  final JCheckBox delRight2CheckBox;

  /* Constructor */
  public FribourgComplementOptionsPanel() {
    this.setLayout(new GridLayout(0,1));
    this.setBorder(new EmptyBorder(10,10,5,10)); // (top,left,bottom,right)

    // MakeComplete option
    makeCompleteCheckBox = new JCheckBox("Make input automaton complete if it is incomplete");
    makeCompleteCheckBox.setSelected(FribourgOptions.getMakeCompletePref());
    this.add(makeCompleteCheckBox);

    // DelRight2 option
    delRight2CheckBox = new JCheckBox("Ignore states whose rightmost component has color 2");
    if (makeCompleteCheckBox.isSelected()) delRight2CheckBox.setSelected(FribourgOptions.getDelRight2Pref());
    else delRight2CheckBox.setEnabled(false);
    this.add(delRight2CheckBox);
    // When makeCompleteCheckBox is selected, enable delRight2Checkbox, and vice versa
    makeCompleteCheckBox.addItemListener(new ItemListener() {
      public void itemStateChanged(ItemEvent e) {
        if (makeCompleteCheckBox.isSelected())
          delRight2CheckBox.setEnabled(true);
        else {
          delRight2CheckBox.setSelected(false);
          delRight2CheckBox.setEnabled(false);
        }
      }
    }); 
  }

  /* Create a FribourgOptions according to the state (checked/unchecked) of the
   * checkboxes. */
  @Override // Abstract method of OptionsPanel
  public FribourgOptions getOptions() {
    Properties props = new Properties();
    props.setProperty(FribourgOptions.getMakeCompleteKey(), makeCompleteCheckBox.isSelected());
    props.setProperty(FribourgOptions.getDelRight2Key(), delRight2CheckBox.isSelected());
    return new FribourgOptions(props);
  }

  /* Set the state of the checkboxes (checked/unchecked) according to the
   * default values in the preference file of the corresonding options. */
  @Override // Abstract method of OptionsPanel
  public void reset() {
    makeCompleteCheckBox.setSelected(FribourgOptions.getMakeCompleteDefault());
    delRight2CheckBox.setSelected(FribourgOptions.getDelRight2Default());
  }
}
