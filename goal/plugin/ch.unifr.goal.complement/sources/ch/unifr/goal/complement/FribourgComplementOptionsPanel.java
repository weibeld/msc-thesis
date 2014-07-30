package ch.unifr.goal.complement;


import org.svvrl.goal.gui.pref.OptionsPanel;
import org.svvrl.goal.core.Properties;
import javax.swing.Box;
import javax.swing.JLabel;
import javax.swing.JCheckBox;
import java.awt.GridLayout;
import javax.swing.border.EmptyBorder;

import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;

/* JPanel > OptionsPanel*/
public class FribourgComplementOptionsPanel extends OptionsPanel<FribourgOptions> {

  final JCheckBox makeCompleteCheckBox;
  final JCheckBox ignoreRightColor2CheckBox;

  public FribourgComplementOptionsPanel() {
    this.setLayout(new GridLayout(0,1));
    this.setBorder(new EmptyBorder(10,10,5,10)); // (top,left,bottom,right)

    // Make automaton complete at start option
    //makeCompleteCheckBox = new JCheckBox("Make input automaton complete at start");
    //makeCompleteCheckBox = new JCheckBox("Ensure input automaton is complete (make complete if incomplete)");
    makeCompleteCheckBox = new JCheckBox("Make input automaton complete if it is incomplete");
    this.add(makeCompleteCheckBox);

    // Ignore states whose rightmost color is 2 option. Disabled on start because
    // gets enabled only if makeCompleteCheckBox is selected.
    ignoreRightColor2CheckBox = new JCheckBox("Ignore states whose rightmost component has color 2");
    ignoreRightColor2CheckBox.setEnabled(false);
    this.add(ignoreRightColor2CheckBox);

    // When makeCompleteCheckBox is selected, enable ignoreRightColor2Checkbox, and vice versa
    makeCompleteCheckBox.addItemListener(new ItemListener() {
      public void itemStateChanged(ItemEvent e) {
        if (makeCompleteCheckBox.isSelected())
          ignoreRightColor2CheckBox.setEnabled(true);
        else {
          ignoreRightColor2CheckBox.setSelected(false);
          ignoreRightColor2CheckBox.setEnabled(false);
        }
      }
    });
    
  }

  /* Return a FribourgOptions object with the options set according to the
   * checkboxes in the panel */
  @Override // Abstract method of OptionsPanel
  public FribourgOptions getOptions() {
    Properties options = new Properties(); // The object to initialize the FribourgOptions object
    options.setProperty(FribourgOptions.makeCompleteKey, makeCompleteCheckBox.isSelected());
    options.setProperty(FribourgOptions.ignoreRightColor2Key, ignoreRightColor2CheckBox.isSelected());
    return new FribourgOptions(options);
  }

  /* From the documentation: Reset options in this panel. Not clear what this
   * method is supposed to do. */
  @Override // Abstract method of OptionsPanel
  public void reset() {
  }
}