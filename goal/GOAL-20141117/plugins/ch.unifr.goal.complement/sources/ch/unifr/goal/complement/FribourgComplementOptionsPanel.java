/*============================================================================*
 * Author: Daniel Weibel <daniel.weibel@unifr.ch>
 * Last modified: 5 Oct. 2014
 *============================================================================*/
package ch.unifr.goal.complement;

import javax.swing.Box;
import javax.swing.JCheckBox;
import javax.swing.border.EmptyBorder;
import java.awt.GridLayout;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import org.svvrl.goal.core.Properties;
import org.svvrl.goal.gui.pref.OptionsPanel;


/*----------------------------------------------------------------------------*
 * The JPanel appearing in the options dialog when complementing an automaton,
 * and in the GOAL preferences under Preferences > Complementation >
 * FribourgConstruction.
 * Contains a checkbox for every option of the Fribourg Construction. Returns a
 * FribourgOptions with options set according to the state of the checkboxes.
 * On object creation, the checkboxes are set according to the preference values
 * in the preference file.
 *----------------------------------------------------------------------------*/
// JPanel > OptionsPanel
public class FribourgComplementOptionsPanel extends OptionsPanel<FribourgOptions> {

  // A checkbox for every option
  private final JCheckBox cCheckBox;
  private final JCheckBox r2ifcCheckBox;
  private final JCheckBox mCheckBox;
  private final JCheckBox m2CheckBox;
  private final JCheckBox bCheckBox;
  private final JCheckBox maccCheckBox;
  private final JCheckBox rCheckBox;
  private final JCheckBox rrCheckBox;

  /* Constructor */
  public FribourgComplementOptionsPanel() {

    // Layout of panel in OptionsDialog
    this.setLayout(new GridLayout(0,1));
    this.setBorder(new EmptyBorder(10,10,5,10)); // (top,left,bottom,right)

    // vBox: vertical stack of hBoxes
    Box vBox = createYBox();
    Box hBox = createXBox();

    

    // Option: component merging optimisation (-m)
    mCheckBox = new JCheckBox("Apply component merging optimisation (-m1)");
    mCheckBox.setSelected(FribourgOptions.getMPref());
    vBox.add(hBox.add(mCheckBox));

    // Option: colour 2 reduction optimisation (-m2). Can only be selected if
    // the -m option is also selected.
    m2CheckBox = new JCheckBox("Apply colour 2 component reduction optimisation (-m2)");
    // If merge optimisation is on, initialise checkbox normally
    if (mCheckBox.isSelected()) m2CheckBox.setSelected(FribourgOptions.getM2Pref());
    // Else, set checkbox unchecked (default) and non-editable
    else m2CheckBox.setEnabled(false);
    vBox.add(hBox.add(m2CheckBox));
    // Whenever merge is ticked, make colour 2 editable, and vice versa
    mCheckBox.addItemListener(new ItemListener() {
      public void itemStateChanged(ItemEvent e) {
        if (mCheckBox.isSelected())
          m2CheckBox.setEnabled(true);
        else {
          m2CheckBox.setSelected(false);
          m2CheckBox.setEnabled(false);
        }
      }
    });

    // Option: rightmost colour 2 pruning if input automaton is complete (-r2ifc)
    r2ifcCheckBox = new JCheckBox("If input automaton is complete, apply rightmost colour 2 optimisation (-r2c)");
    r2ifcCheckBox.setSelected(FribourgOptions.getR2ifcPref());
    vBox.add(hBox.add(r2ifcCheckBox));

    // Option: make input automaton complete (-c)
    cCheckBox = new JCheckBox("Make input automaton complete if it is incomplete (-c)");
    cCheckBox.setSelected(FribourgOptions.getCPref());
    vBox.add(hBox.add(cCheckBox));

    // Option: maximise the accepting states of the input automaton
    maccCheckBox = new JCheckBox("Maximise the accepting set of the input automaton (-macc)");
    maccCheckBox.setSelected(FribourgOptions.getMaccPref());
    vBox.add(hBox.add(maccCheckBox));

    // Option: prune unreachable and dead states from the output automaton
    rCheckBox = new JCheckBox("Remove unreachable and dead states from the output automaton (-r)");
    rCheckBox.setSelected(FribourgOptions.getRPref());
    vBox.add(hBox.add(rCheckBox));

    // Option: prune unreachable and dead states from the input automaton
    rrCheckBox = new JCheckBox("Remove unreachable and dead states from the input automaton (-rr)");
    rrCheckBox.setSelected(FribourgOptions.getRRPref());
    vBox.add(hBox.add(rrCheckBox));

    // Option: use the bracket notation
    bCheckBox = new JCheckBox("Use the \"bracket notation\" for labelling states (-b)");
    bCheckBox.setSelected(FribourgOptions.getBPref());
    vBox.add(hBox.add(bCheckBox));

    // Add the vertical stack of hBoxes
    this.add(vBox);
  }

  /* Create a FribourgOptions according to the state of the checkboxes */
  @Override // Abstract method of OptionsPanel
  public FribourgOptions getOptions() {
    FribourgOptions options = new FribourgOptions();
    options.setC(cCheckBox.isSelected());
    options.setR2ifc(r2ifcCheckBox.isSelected());
    options.setM(mCheckBox.isSelected());
    options.setM2(m2CheckBox.isSelected());
    options.setB(bCheckBox.isSelected());
    options.setMacc(maccCheckBox.isSelected());
    options.setR(rCheckBox.isSelected());
    options.setRR(rrCheckBox.isSelected());
    return options;
  }

  /* Set the checkboxes according to the option default values, which are in
   * the preference file. Used when the user clicks on 'Reset' in 'Preferences..
   * > Complementation > Fribourg Construction' (currently not implemented */
  @Override // Abstract method of OptionsPanel
  public void reset() {
    cCheckBox.setSelected(FribourgOptions.getCDefault());
    r2ifcCheckBox.setSelected(FribourgOptions.getR2ifcDefault());
    mCheckBox.setSelected(FribourgOptions.getMDefault());
    m2CheckBox.setSelected(FribourgOptions.getM2Default());
    bCheckBox.setSelected(FribourgOptions.getBDefault());
    maccCheckBox.setSelected(FribourgOptions.getMaccDefault());
    rCheckBox.setSelected(FribourgOptions.getRDefault());
    rrCheckBox.setSelected(FribourgOptions.getRRDefault());
  }
}
