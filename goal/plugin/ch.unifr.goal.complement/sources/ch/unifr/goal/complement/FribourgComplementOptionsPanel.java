package ch.unifr.goal.complement;

import javax.swing.Box;
import javax.swing.JCheckBox;
import java.awt.GridLayout;
import javax.swing.border.EmptyBorder;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import org.svvrl.goal.gui.pref.OptionsPanel;
import org.svvrl.goal.core.Properties;


/*----------------------------------------------------------------------------*
 * The panel in the options dialog in the GUI. Contains a checkbox for every
 * option of the FribourgConstruction. Returns a FribourgOptions that reflects
 * the selection of checkboxes in the panel. The checkboxes are initially set
 * according to the pfererence values saved in the Preference file.
 * Daniel Weibel, 31.07.2014
 *----------------------------------------------------------------------------*/

// JPanel > OptionsPanel
public class FribourgComplementOptionsPanel extends OptionsPanel<FribourgOptions> {

  // A checkbox for every option
  private final JCheckBox complCheckBox;
  private final JCheckBox right2IfComplCheckBox;
  private final JCheckBox mergeCheckBox;
  private final JCheckBox reduce2CheckBox;
  private final JCheckBox brackCheckBox;

  /* Constructor */
  public FribourgComplementOptionsPanel() {

    // Layout of panel in OptionsDialog
    this.setLayout(new GridLayout(0,1));
    this.setBorder(new EmptyBorder(10,10,5,10)); // (top,left,bottom,right)

    // vBox: vertical stack of hBoxes
    Box vBox = createYBox();
    Box hBox = createXBox();

    // Make input automaton complete
    complCheckBox = new JCheckBox("Make input automaton complete if it is incomplete");
    complCheckBox.setSelected(FribourgOptions.getComplPref());
    vBox.add(hBox.add(complCheckBox));

    // Rightmost colour 2 optmisation if input automaton is complete
    right2IfComplCheckBox = new JCheckBox("If input automaton is complete, apply rightmost colour 2 optimisation");
    right2IfComplCheckBox.setSelected(FribourgOptions.getRight2IfComplPref());
    vBox.add(hBox.add(right2IfComplCheckBox));

    // Component merging optimisation
    mergeCheckBox = new JCheckBox("Apply component merging optimisation");
    mergeCheckBox.setSelected(FribourgOptions.getMergePref());
    vBox.add(hBox.add(mergeCheckBox));

    // Colour 2 reduction optimisation. Can only be selected if the component
    // merging optimisation is also selected.
    reduce2CheckBox = new JCheckBox("Apply colour 2 reduction optimisation");
    // If merge optimisation is on, initialise checkbox normally
    if (mergeCheckBox.isSelected()) reduce2CheckBox.setSelected(FribourgOptions.getReduce2Pref());
    // Else, set checkbox unchecked (default) and non-editable
    else reduce2CheckBox.setEnabled(false);
    vBox.add(hBox.add(reduce2CheckBox));
    // Whenever merge is ticked, make colour 2 editable, and vice versa
    mergeCheckBox.addItemListener(new ItemListener() {
      public void itemStateChanged(ItemEvent e) {
        if (mergeCheckBox.isSelected())
          reduce2CheckBox.setEnabled(true);
        else {
          reduce2CheckBox.setSelected(false);
          reduce2CheckBox.setEnabled(false);
        }
      }
    });

    // Use the bracket notation
    brackCheckBox = new JCheckBox("Use the \"bracket notation\" for state labels");
    brackCheckBox.setSelected(FribourgOptions.getBrackPref());
    vBox.add(hBox.add(brackCheckBox));

    // Add the vertical stack of hBoxes
    this.add(vBox);
  }

  /* Create a FribourgOptions according to the state of the checkboxes */
  @Override // Abstract method of OptionsPanel
  public FribourgOptions getOptions() {
    FribourgOptions options = new FribourgOptions();
    options.setCompl(complCheckBox.isSelected());
    options.setRight2IfCompl(right2IfComplCheckBox.isSelected());
    options.setMerge(mergeCheckBox.isSelected());
    options.setReduce2(reduce2CheckBox.isSelected());
    options.setBrack(brackCheckBox.isSelected());
    return options;
  }

  /* Set the checkboxes according to the option default values, which are in
   * the preference file. Used when the user clicks on 'Reset' in 'Preferences..
   * > Complementation > Fribourg Construction' (currently not implemented */
  @Override // Abstract method of OptionsPanel
  public void reset() {
    complCheckBox.setSelected(FribourgOptions.getComplDefault());
    right2IfComplCheckBox.setSelected(FribourgOptions.getRight2IfComplDefault());
    mergeCheckBox.setSelected(FribourgOptions.getMergeDefault());
    reduce2CheckBox.setSelected(FribourgOptions.getReduce2Default());
    brackCheckBox.setSelected(FribourgOptions.getBrackDefault());
  }
}
