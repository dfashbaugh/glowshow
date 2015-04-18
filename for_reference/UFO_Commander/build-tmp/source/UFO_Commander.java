import processing.core.*; 
import processing.data.*; 
import processing.event.*; 
import processing.opengl.*; 

import controlP5.*; 
import oscP5.*; 
import netP5.*; 
import processing.serial.*; 
import java.util.LinkedList; 
import java.util.regex.Matcher; 
import java.util.regex.Pattern; 

import java.util.HashMap; 
import java.util.ArrayList; 
import java.io.BufferedReader; 
import java.io.PrintWriter; 
import java.io.InputStream; 
import java.io.OutputStream; 
import java.io.IOException; 

public class UFO_Commander extends PApplet {




 






final boolean DEBUG = true; // osc
final boolean DEBUG_SERIAL = false;


final String PRESET_FILE = "presets.txt";
final String SERIAL_PORT = "/dev/tty.usbserial-A501E2IQ";

//final String SERIAL_PORT = "/dev/tty.usbmodem1d11";

final int INITIAL_PATTERN = 80;

String IPAD_ADDRESS = "10.0.2.2";

//String IPAD_ADDRESS = "169.254.248.70";

int IPAD_PORT = 8000;
int MY_PORT = 12001;

final int BAUD_RATE = 9600;
final int MESSAGE_SIZE = 14;
final int TIMING_ADDR = 129;
final int OFF_PATTERN = 68;
final int DELIMETER = 128;
final int INTERVAL = 100;

int lastHeartbeat;

boolean stealth = false;

ArrayList lightGroups = new ArrayList();
ArrayList presets = new ArrayList();

LightGroup groupAll;
// LightGroup groupHorizontalPoles;
// LightGroup groupVerticalPoles;
LightGroup groupBoxes;
// LightGroup groupDMX;
LightGroup groupPoles;

// ControlP5

final int LIGHT_GROUP_WIDTH = 270;
final int RADIO_SIZE = 20;
final int PADDING = 15;

ControlP5 controlP5;

String[] patterns = new String[127];
String[] mappings = new String[6];

ListBox presetList;
Textfield presetNamer;

boolean sympathizeEvents = false;

Serial port;
LinkedList messageQueue = new LinkedList();

// OSCP5

OscP5 oscP5;
NetAddress myRemoteLocation;
NetAddress myCompLocation;


// Map of which groups to address.
// In order of the lightGroups array.
// Object cuz oscP5.
// Object[] activeAddr = { 1.0, 0.0, 0.0, 0.0, 0.0 };
Object[] activeAddr = { 1.0f, 0.0f, 0.0f };

int[] patternsToIndeces;

public void setup() {
  
  Matcher m = Pattern.compile("/preset/(\\d+)").matcher("/preset/23");

  m.find();
  println(m.group(1));



  size(830, 700);

  patterns[80] = "pulseOnce";
  patterns[79] = "colorWipeMeterGradient";
  patterns[78] = "colorWipeMeter";
  patterns[77] = "colorChase";
  patterns[76] = "stripe";
  patterns[75] = "colorAlternator";
  patterns[74] = "colorWipe";
  patterns[73] = "bounce";
  patterns[72] = "pulseSaw";
  patterns[71] = "pulseSine";
  patterns[70] = "gradient";
  patterns[69] = "solidColor";
  patterns[OFF_PATTERN] = "off";
  patterns[66] = "rainbow";
  patterns[65] = "rainbowCycle";
  patterns[64] = "totesRandom";
  patterns[63] = "flickerStrobeFour";
  patterns[62] = "flickerStrobeTwo";

  mappings[1] = "forward";
  mappings[2] = "backward";
  mappings[3] = "peak";
  mappings[4] = "valley";
  mappings[5] = "dither";

  controlP5 = new ControlP5(this);

  // Only instantiate port if we know for sure it exists.
  // Port won't be null if you give it a bunk address.
  String[] serialPorts = Serial.list();
  println (serialPorts);
  for (int i = 0; i < serialPorts.length; i++) {
    if (serialPorts[i].equals(SERIAL_PORT)) {
      port = new Serial(this, SERIAL_PORT, BAUD_RATE);  
    }
  }
  if (port != null) {
    println("Using serial port " + SERIAL_PORT);
  } else { 
    println("Did not find a serial port!");
  }

  // Think of this as push/pop/sympathizeEvents
  // first run of controlEvent causes probs when not all 
  // controllers have been created.
  sympathizeEvents = true;

  groupAll = new LightGroup("All", 0) ;
  // groupHorizontalPoles = new LightGroup("Horizontal Poles", 0);
  // groupVerticalPoles = new LightGroup("Vertical Poles", 7);
  groupPoles = new LightGroup("Poles", 1);
  groupBoxes = new LightGroup("Boxes", 2);

  sympathizeEvents = false;



  int x = lightGroups.size() * LIGHT_GROUP_WIDTH + PADDING;
  
  presetNamer = controlP5.addTextfield("preset-namer")
                         .setPosition(PADDING, PADDING*2)
                         .setLabel("Save preset.")
                         .setSize(LIGHT_GROUP_WIDTH, 20)
                         .moveTo("presets");

  presetList = controlP5.addListBox("preset-list")
                        .setPosition(PADDING, PADDING*2 + 70)
                        .setSize(LIGHT_GROUP_WIDTH, 600)
                        .setItemHeight(20)
                        .actAsPulldownMenu(false)
                        .moveTo("presets");

  oscP5 = new OscP5(this, MY_PORT);
  myRemoteLocation = new NetAddress(IPAD_ADDRESS, IPAD_PORT);
  myCompLocation = new NetAddress(oscP5.ip(), MY_PORT);

  loadPresets();
  synchronizePatterns();
  synchronizeMappings();

}

public void draw() {
 
  background(0);

  if (port != null) heartbeat();

  // checks for the existence of port itself
  // still debugs if port isn't there
  if (!stealth) emptyMessageQueue();

}

public void sendAllMessages() {
  for (int i = 0; i < lightGroups.size(); i++) {
    LightGroup l = (LightGroup)lightGroups.get(i);
    l.sendMessage();
  }
}

public void emptyMessageQueue() {

  while (messageQueue.peek() != null) {
    byte b = ((Byte)messageQueue.poll()).byteValue();
    if (port != null) port.write(b);
    if (DEBUG_SERIAL) println("[SERIAL] " + b);
  }

}

public void heartbeat() {
  int now = millis();
  if (now - lastHeartbeat >= INTERVAL) {
    port.write(TIMING_ADDR);
    port.write(new Integer(now).toString());
    port.write(DELIMETER);
    lastHeartbeat = now;
  }
}

public void keyPressed() {
  if (key == ' ')
    ((LightGroup)lightGroups.get(1)).sendMessage();
    ((LightGroup)lightGroups.get(2)).sendMessage();
}

public void controlEvent(ControlEvent theEvent) {

  LightGroup l = checkLightControllers(theEvent);

  if (keyPressed || l == groupAll) expressSympathy(theEvent);

  if (theEvent.isFrom(presetNamer)) {
    savePreset(theEvent.getStringValue());
    return;
  }

  if (theEvent.isFrom(presetList)) {
    applyPreset((int)theEvent.value());
  }

}

public void expressSympathy(ControlEvent theEvent) {

  if (!sympathizeEvents) {
    
    sympathizeEvents = true;

    String name = theEvent.name();
    name = name.substring(0, name.indexOf("-"));

    for (Object o : lightGroups) {

      LightGroup l = (LightGroup)o;
      String addr = name + "-" + l.address;
      Controller c = controlP5.getController(addr);
      
      // Controller groups are weird.
      if (theEvent.isGroup() && theEvent.getGroup() instanceof ColorPicker) {
        
        if (name.equals("picker1")) {
          l.setColor1((int)theEvent.value());
        } else if (name.equals("picker2")) {
          l.setColor2((int)theEvent.value());
        }

      } else if (theEvent.isGroup() && theEvent.getGroup() instanceof RadioButton) {

        if (name.equals("patterns")) {
          l.setPattern((int)theEvent.value());
        } else if (name.equals("mappings")) { 
          l.setMapping((int)theEvent.value());
        }

      } else if (c != null) {

        c.setValue(theEvent.value());

      }


    }

    sympathizeEvents = false;

  }

}

/**
 * Returns Light group controller came from or null if none.
 */
public LightGroup checkLightControllers(ControlEvent theEvent) {

  for (Object o : lightGroups) {

    LightGroup l = (LightGroup)o;

    // h8 u cp5
    // Would have liked to do all listeners, but I couldn't, because there
    // are controller groups. Just did it all here to be consistent.

    if (theEvent.isFrom(l.bang)) {
      l.sendMessage();
      return l;
    }

    if (theEvent.isFrom(l.rateSlider)) {
      l.rate = (int)theEvent.value();
      return l;
    }

    if (theEvent.isFrom(l.brightnessSlider)) {
      l.brightness = (int)theEvent.value();
      return l;
    }

    if (theEvent.isFrom(l.patternList)) {
      l.pattern = (int)theEvent.value();
      return l;
    }

    if (theEvent.isFrom(l.mappingList)) {
      l.mapping = (int)theEvent.value();
      return l;
    }

    if (theEvent.isFrom(l.colorPicker1)) {
      l.color1 = (int)theEvent.value();
      return l;
    }

    if (theEvent.isFrom(l.colorPicker2)) {
      l.color2 = (int)theEvent.value();
      return l;
    }

  }

  return null;

}

public void applyPreset(int presetIndex) {

  LightGroupSettings[] preset = (LightGroupSettings[])presets.get(presetIndex);
  
  for (int i = 0; i < lightGroups.size(); i++) {
    LightGroup l = (LightGroup)lightGroups.get(i);
    l.applySettings(preset[i]);
  }

  sendAllMessages();

}

public void savePreset(String presetName) {

  LightGroupSettings[] preset = new LightGroupSettings[lightGroups.size()];

  for (int i = 0; i < lightGroups.size(); i++) {
    LightGroup l = (LightGroup)lightGroups.get(i);
    preset[i] = l.getSettings();
  }

  presets.add(preset);
  presetList.addItem(presetName, presets.size()-1);

  writePresets();

}

public void writePresets() {

  PrintWriter output = createWriter(PRESET_FILE);

  for (int i = 0; i < presets.size(); i++) {

    LightGroupSettings[] l = (LightGroupSettings[])presets.get(i);
    String[] serialized = new String[l.length];

    String name = presetList.getItem(i).getName();

    for (int j = 0; j < serialized.length; j++) {
      serialized[j] = l[j].serialize();
    }

    output.println(name + ":" + join(serialized, ":"));

  }

  output.flush();
  output.close();

  synchronizePresets();

}

public void loadPresets() {

  BufferedReader reader = createReader(PRESET_FILE);

  String line = null;

  do { 
    
    try {
      line = reader.readLine();
    } catch (IOException e) {
      e.printStackTrace();
      line = null;
    }

    if (line != null) {
      parsePreset(line);
    }

  } while (line != null);

  synchronizePresets();

}

public void synchronizePresets() {

  String[] s = new String[presets.size()];
  for (int i = 0; i < s.length; i++) {
    s[i] = presetList.getItem(i).getName();
  }

  OscMessage message = new OscMessage("/Presets/setLabels");
  message.add(s);
  oscP5.send(message, myRemoteLocation);

}

public void synchronizePatterns() {

  // count non-null presets
  int numPatterns = 0;
  for (int i = 0; i < patterns.length; i++) {
    if (patterns[i] != null) {
      numPatterns++;
    }
  }

  patternsToIndeces = new int[numPatterns];

  // make arr
  String[] s = new String[numPatterns];
  int j = 0;
  for (int i = 0; i < patterns.length; i++) {
    if (patterns[i] != null) {
      s[j] = patterns[i];
      patternsToIndeces[j] = i;
      j++;
    }
  }

  OscMessage message = new OscMessage("/Patterns/setLabels");
  message.add(s);
  oscP5.send(message, myRemoteLocation);

}

public void synchronizeMappings() {

  // count non-null presets
  int numMappings = 0;
  for (int i = 0; i < mappings.length; i++) {
    if (mappings[i] != null) {
      numMappings++;
    }
  }

  // make arr
  String[] s = new String[numMappings];
  int j = 0;
  for (int i = 0; i < mappings.length; i++) {
    if (mappings[i] != null) {
      s[j] = mappings[i];
      j++;
    }
  }

  OscMessage message = new OscMessage("/Mappings/setLabels");
  message.add(s);
  oscP5.send(message, myRemoteLocation);

}

public void parsePreset(String s) {
  
  String[] data = split(s, ":");

  LightGroupSettings[] preset = new LightGroupSettings[data.length-1];

  for (int i = 0; i < preset.length; i++) {
    preset[i] = new LightGroupSettings(data[i+1]);
  }

  presets.add(preset);
  presetList.addItem(data[0], presets.size()-1);

}
class LemurReader implements OscReader {


  public void read(OscMessage theOscMessage) {

    if (theOscMessage.addrPattern().equals("/Presets/x")) {
      
      for (int i = 0; i < theOscMessage.arguments().length; i++) {
        if (theOscMessage.arguments()[i].equals(1.0f)) {
    
          applyPreset(i);
          sendFeedback();

        }
      }

    } else if (theOscMessage.addrPattern().equals("/Patterns/x")) {
      
      for (int i = 0; i < theOscMessage.arguments().length; i++) {
        if (theOscMessage.arguments()[i].equals(1.0f)) {
        
          for (int j = 0; j < activeAddr.length; j++) {
            if (activeAddr[j].equals(1.0f)) {
              
              LightGroup l = (LightGroup)lightGroups.get(j);
              l.setPattern(patternsToIndeces[i]);
              l.sendMessage();

            }
          }

        }
      }

    } else if (theOscMessage.addrPattern().equals("/Mappings/x")) {
      
      for (int i = 0; i < theOscMessage.arguments().length; i++) {
        if (theOscMessage.arguments()[i].equals(1.0f)) {
        
          for (int j = 0; j < activeAddr.length; j++) {
            if (activeAddr[j].equals(1.0f)) {
              
              LightGroup l = (LightGroup)lightGroups.get(j);
              l.setMapping(i+1); // hack.
              l.sendMessage();

            }
          }

        }
      }

    } else if (theOscMessage.addrPattern().equals("/Addr/x")) { 


      setActiveAddr(theOscMessage.arguments());
      sendFeedback();
      

    } else if (theOscMessage.addrPattern().equals("/Stealth/x")) {

      if (theOscMessage.arguments()[0].equals(1.0f)) {
      
        stealth = true;
      
      } else { 
        
        stealth = false;

        for (int j = 0; j < activeAddr.length; j++) {
          if (activeAddr[j].equals(1.0f)) {
            LightGroup l = (LightGroup)lightGroups.get(j);
            l.sendMessage();
          }
        }

      }

    } else if (theOscMessage.addrPattern().equals("/RedSlider1/x")) { 

      for (int i = 0; i < activeAddr.length; i++) {
        if (activeAddr[i].equals(1.0f)) {

          int v = (int)(theOscMessage.get(0).floatValue() * 255);
          LightGroup l = (LightGroup)lightGroups.get(i);
          l.setColor1(v, l.g1, l.b1);
          l.sendMessage();


        }
      }

    } else if (theOscMessage.addrPattern().equals("/GreenSlider1/x")) { 

      for (int i = 0; i < activeAddr.length; i++) {
        if (activeAddr[i].equals(1.0f)) {

          int v = (int)(theOscMessage.get(0).floatValue() * 255);
          LightGroup l = (LightGroup)lightGroups.get(i);
          l.setColor1(l.r1, v, l.b1);
          l.sendMessage();


        }
      }

    } else if (theOscMessage.addrPattern().equals("/BlueSlider1/x")) { 

      for (int i = 0; i < activeAddr.length; i++) {
        if (activeAddr[i].equals(1.0f)) {

          int v = (int)(theOscMessage.get(0).floatValue() * 255);
          LightGroup l = (LightGroup)lightGroups.get(i);
          l.setColor1(l.r1, l.g1, v);
          l.sendMessage();


        }
      }

    } else if (theOscMessage.addrPattern().equals("/RedSlider2/x")) { 

      for (int i = 0; i < activeAddr.length; i++) {
        if (activeAddr[i].equals(1.0f)) {

          int v = (int)(theOscMessage.get(0).floatValue() * 255);
          LightGroup l = (LightGroup)lightGroups.get(i);
          l.setColor2(v, l.g2, l.b2);
          l.sendMessage();


        }
      }

    } else if (theOscMessage.addrPattern().equals("/GreenSlider2/x")) { 

      for (int i = 0; i < activeAddr.length; i++) {
        if (activeAddr[i].equals(1.0f)) {

          int v = (int)(theOscMessage.get(0).floatValue() * 255);
          LightGroup l = (LightGroup)lightGroups.get(i);
          l.setColor2(l.r2, v, l.b2);
          l.sendMessage();


        }
      }

    } else if (theOscMessage.addrPattern().equals("/BlueSlider2/x")) { 

      for (int i = 0; i < activeAddr.length; i++) {
        if (activeAddr[i].equals(1.0f)) {

          int v = (int)(theOscMessage.get(0).floatValue() * 255);
          LightGroup l = (LightGroup)lightGroups.get(i);
          l.setColor2(l.r2, l.g2, v);
          l.sendMessage();


        }
      }

    } else if (theOscMessage.addrPattern().equals("/RateSlider/x")) { 

      for (int i = 0; i < activeAddr.length; i++) {
        if (activeAddr[i].equals(1.0f)) {

          int v = (int)(theOscMessage.get(0).floatValue() * 127);
          LightGroup l = (LightGroup)lightGroups.get(i);
          l.setRate(v);
          l.sendMessage();

        }
      }

    } else if (theOscMessage.addrPattern().equals("/BrightnessSlider/x")) { 

      for (int i = 0; i < activeAddr.length; i++) {
        if (activeAddr[i].equals(1.0f)) {

          int v = (int)(theOscMessage.get(0).floatValue() * 127);
          LightGroup l = (LightGroup)lightGroups.get(i);
          l.setBrightness(v);
          l.sendMessage();

        }
      }

    }
    
  }


  public void sendFeedback() {

    int feedbackIndex = 0;

    for (int i = 0; i < activeAddr.length; i++) {
      if (activeAddr[i].equals(1.0f)) {
        feedbackIndex = i;
        break;
      }
    }

    LightGroup l = (LightGroup)lightGroups.get(feedbackIndex);

    oscMessage("/RedSlider1/x",   map(red(l.color1), 0, 255, 0, 1));
    oscMessage("/GreenSlider1/x", map(green(l.color1), 0, 255, 0, 1));
    oscMessage("/BlueSlider1/x",  map(blue(l.color1), 0, 255, 0, 1));

    oscMessage("/RedSlider2/x",   map(red(l.color2), 0, 255, 0, 1));
    oscMessage("/GreenSlider2/x", map(green(l.color2), 0, 255, 0, 1));
    oscMessage("/BlueSlider2/x",  map(blue(l.color2), 0, 255, 0, 1));

    oscMessage("/RateSlider/x",   map(l.rate, 0, 127, 0, 1));
    oscMessage("/BrightnessSlider/x", map(l.brightness, 0, 127, 0, 1));

    // Send pattern message.
    
    int patternIndex = 0;
    for (int i = 0; i < patternsToIndeces.length; i++) {
      if (patternsToIndeces[i] == l.pattern) {
        patternIndex = i;
      }
    }

    float[] patternMessage = new float[patternsToIndeces.length];
    for (int i = 0; i < patternMessage.length; i++) {
      if (i == patternIndex) {
        patternMessage[i] = 1.0f;
      }
    }

    OscMessage message = new OscMessage("/Patterns/x");
    message.add(patternMessage);
    oscP5.send(message, myRemoteLocation);

    // Send mapping message.

    int mappingIndex = l.mapping - 1; // HACK
    message = new OscMessage("/Mappings/x");
    float[] mappingMessage = new float[mappings.length];
    for (int i = 0; i < mappings.length; i++) {
      if (i == mappingIndex) {
        mappingMessage[i] = 1.0f;
      }
    }
    message.add(mappingMessage);
    oscP5.send(message, myRemoteLocation);

  }


}
class LightGroup {

  public final int address;

  // These are only public so cp5 can manipulate them!
  // PLZ Don't change manually! (use the setXxxx methods instead)
  public int pattern = OFF_PATTERN;
  public int mapping = 1;
  public int rate = 0;
  public int brightness = 127;

  // Would have liked to make these arrays, but controlP5's reflection makes that harder.
  // Also addListener didn't work for controlGroups :(
  // DONT TOUCH
  public int color1 = color(0), color2 = color(0);

  public int r1, r2, g1, g2, b1, b2;

  // Interface elements
  public final ColorPicker colorPicker1, colorPicker2;
  public final RadioButton patternList;
  public final RadioButton mappingList;
  public final Slider rateSlider;
  public final Slider brightnessSlider;
  public final Bang bang;

  // Just the order in which we created this group
  protected final int index;

  public LightGroup(String title, int address) {

    index = lightGroups.size();
    lightGroups.add(this);
    this.address = address;

    // Build interface

    int SEND_MESSAGE_HEIGHT = 30;
    int x = index * LIGHT_GROUP_WIDTH + PADDING;
    int y = PADDING*2;

    bang = controlP5.addBang("bang-"+address)
             .setPosition(x, y)
             .setSize(LIGHT_GROUP_WIDTH-PADDING, SEND_MESSAGE_HEIGHT)
             .setTriggerEvent(Bang.RELEASE)
             .setLabel(title);

    y += SEND_MESSAGE_HEIGHT + PADDING*1.5f;

    colorPicker1 = controlP5.addColorPicker("picker1-" + address)
                            .setPosition(x, y)
                            .setColorValue(color1);
    y += 70;

    colorPicker2 = controlP5.addColorPicker("picker2-" + address)
                            .setPosition(x, y)
                            .setColorValue(color2);
    y += 70;

    rateSlider = controlP5.addSlider("rate-"+address)
             .setPosition(x, y)
             .setRange(0, 127)
             .setSize(220, 20)
             .setDecimalPrecision(0)
             .setLabel("rate")
             .setValue(rate);

    y += PADDING * 2;

    brightnessSlider = controlP5.addSlider("brightness-"+address)
             .setPosition(x, y)
             .setRange(0, 127)
             .setSize(220, 20)
             .setDecimalPrecision(0)
             .setLabel("bright")
             .setValue(brightness);

    y += PADDING * 2;
    
    patternList = controlP5.addRadioButton("patterns-" + address)
                           .setPosition(x , y)
                           .setSize(RADIO_SIZE, RADIO_SIZE)
                           .setNoneSelectedAllowed(false);

    int j = 0;
    for (int i = 0; i < patterns.length; i++) {
      if (patterns[i] != null) {
        patternList.addItem(patterns[i] + address, i);
        patternList.getItem(j).setCaptionLabel(patterns[i]);
        if (i == INITIAL_PATTERN) {
          patternList.activate(patternList.getItems().size()-1);
        }
        j++;
      }
    }

    mappingList = controlP5.addRadioButton("mappings-" + address)
                           .setPosition(x +150, y)
                           .setSize(RADIO_SIZE, RADIO_SIZE)
                           .setNoneSelectedAllowed(false);

    j = 0;
    for (int i = 0; i < mappings.length; i++) {
      if (mappings[i] != null) {
        mappingList.addItem(mappings[i] + address, i);
        mappingList.getItem(j).setCaptionLabel(mappings[i]);
        j++;
      }
    }

    mappingList.activate(0);


  }

  public LightGroupSettings getSettings() {

    return new LightGroupSettings(pattern, mapping, rate, brightness, color1, color2);

  }

  public void applySettings(LightGroupSettings settings) {
    setRate(settings.rate);
    setBrightness(settings.brightness);
    setMapping(settings.mapping);
    setPattern(settings.pattern);
    setColor1(settings.color1);
    setColor2(settings.color2);
  }

  public void setRate(int value) {
    rateSlider.setValue(value);      
  }

  public void setBrightness(int value) {
    brightnessSlider.setValue(value);
  }

  public void setMapping(int value) {
    mappingList.activate(mappings[value] + address); // h8 u cp5
  }

  public void setPattern(int value) {
    patternList.activate(patterns[value] + address); // h8 u cp5
  }

  public void setColor1(int r, int g, int b) {
    r1 = r;
    g1 = g;
    b1 = b;
    colorPicker1.setColorValue(color(r1, g1, b1));
  }

  public void setColor2(int r, int g, int b) {
    r2 = r;
    g2 = g;
    b2 = b;
    colorPicker2.setColorValue(color(r2, g2, b2));
  }

  public void setColor1(int c) {
    setColor1((int)red(c), (int)green(c), (int)blue(c));
  }

  public void setColor2(int c) {
    setColor2((int)red(c), (int)green(c), (int)blue(c));
  }

  public void sendMessage() {

    // Unfortunately, mappings go in the pattern slot
    // We must send two "messages"
    sendPatternMessage(pattern);
    sendPatternMessage(mapping);

  }

  private void sendPatternMessage(int pattern) {

    byte[] serialData = new byte[14];
    int color1 = colorPicker1.getColorValue();
    int color2 = colorPicker2.getColorValue();
    float a;

    serialData[0] = (byte)address;
    serialData[1] = (byte)rate;
    serialData[2] = (byte)pattern;

    a = alpha(color1)/255;
    serialData[3] = (byte)(red(color1)   /2*a);
    serialData[4] = (byte)(green(color1) /2*a);
    serialData[5] = (byte)(blue(color1)  /2*a);

    a = alpha(color2)/255;
    serialData[6] = (byte)(red(color2)   /2*a);
    serialData[7] = (byte)(green(color2) /2*a);
    serialData[8] = (byte)(blue(color2)  /2*a);

    // Ignore third color.
    serialData[9] = 0;
    serialData[10] = 0;
    serialData[11] = 0;

    serialData[12] = (byte)brightness;
    serialData[13] = (byte)DELIMETER;

    for (int i = 0; i < serialData.length; i++) {
      messageQueue.offer(serialData[i]);
    }

  }

}
class LightGroupSettings { 

  public final int color1, color2;
  public final int rate;
  public final int pattern;
  public final int mapping;
  public final int brightness;

  public LightGroupSettings(int pattern, int mapping, int rate, int brightness, int color1, int color2) {
    this.pattern = pattern;
    this.mapping = mapping;
    this.rate = rate;
    this.brightness = brightness;
    this.color1 = color1;
    this.color2 = color2;
  }

  public LightGroupSettings(String serialized) {
    String[] data = split(serialized, ",");
    this.pattern = PApplet.parseInt(data[0]);
    this.mapping = PApplet.parseInt(data[1]);
    this.rate = PApplet.parseInt(data[2]);
    this.brightness = PApplet.parseInt(data[3]);
    this.color1 = PApplet.parseInt(data[4]);
    this.color2 = PApplet.parseInt(data[5]);
  }

  public String toString() {

    return "Pattern: " + patterns[pattern] + "\n" + 
           "Mapping: " + mappings[mapping] + "\n" + 
           "Rate: " + rate + "\n" + 
           "Brightness: " + brightness + "\n" + 
           "Color1: " + color1 + "\n" + 
           "Color2: " + color2;

  }

  public String serialize() {
    return pattern + "," + mapping + "," + rate + "," + brightness + "," + color1 + "," + color2;
  }


}

class LiveReader implements OscReader {

  public void read(OscMessage theOscMessage) {

    String p = theOscMessage.addrPattern();

    Matcher presets = Pattern.compile("/preset/(\\d+)").matcher(p);
    Matcher patterns = Pattern.compile("/pattern/(\\d+)").matcher(p);
    Matcher mappings = Pattern.compile("/mapping/(\\d+)").matcher(p);
    Matcher address = Pattern.compile("/address/(\\d+)").matcher(p);
    
    if (presets.find() && theOscMessage.get(0).intValue() == 1) {
    
      applyPreset(parseInt(presets.group(1)));

    } else if (patterns.find() && theOscMessage.get(0).intValue() == 1) {
      
      for (int j = 0; j < activeAddr.length; j++) {
        if (activeAddr[j].equals(1.0f)) {
          
          LightGroup l = (LightGroup)lightGroups.get(j);
          l.setPattern(patternsToIndeces[parseInt(patterns.group(1))]);
          l.sendMessage();

        }
      }

    } else if (mappings.find() && theOscMessage.get(0).intValue() == 1) {
      
      for (int j = 0; j < activeAddr.length; j++) {
        if (activeAddr[j].equals(1.0f)) {
          
          LightGroup l = (LightGroup)lightGroups.get(j);
          l.setMapping(parseInt(mappings.group(1))+1); // hack.
          l.sendMessage();

        }
      }

    } else if (address.find() && theOscMessage.get(0).intValue() == 1) { 

      int a = parseInt(address.group(1));
      for (int i = 0; i < activeAddr.length; i++) {
        if (a == i) {
          activeAddr[i] = 1.0f;
        } else {
          activeAddr[i] = 0.0f;          
        }
      }
      
    } 

    else if (theOscMessage.addrPattern().equals("/stealth/x")) { 


      if (theOscMessage.get(0).intValue() == 1) {
      
        stealth = true;
        
      
      } else { 
        
        stealth = false;

        for (int j = 0; j < activeAddr.length; j++) {
          if (activeAddr[j].equals(1.0f)) {
            LightGroup l = (LightGroup)lightGroups.get(j);
            l.sendMessage();
          }
        }

      }
} 
  else if (p.equals("/red1")) { 

      for (int i = 0; i < activeAddr.length; i++) {
        if (activeAddr[i].equals(1.0f)) {

          int v = (int)(theOscMessage.get(0).floatValue() * 255);
          LightGroup l = (LightGroup)lightGroups.get(i);
          l.setColor1(v, l.g1, l.b1);
          l.sendMessage();

        }
      }

    } else if (p.equals("/green1")) { 

      for (int i = 0; i < activeAddr.length; i++) {
        if (activeAddr[i].equals(1.0f)) {

          int v = (int)(theOscMessage.get(0).floatValue() * 255);
          LightGroup l = (LightGroup)lightGroups.get(i);
          l.setColor1(l.r1, v, l.b1);
          l.sendMessage();

        }
      }

    } else if (p.equals("/blue1")) { 

      for (int i = 0; i < activeAddr.length; i++) {
        if (activeAddr[i].equals(1.0f)) {

          int v = (int)(theOscMessage.get(0).floatValue() * 255);
          LightGroup l = (LightGroup)lightGroups.get(i);
          l.setColor1(l.r1, l.g1, v);
          l.sendMessage();


        }
      }

    } else if (p.equals("/red2")) { 

      for (int i = 0; i < activeAddr.length; i++) {
        if (activeAddr[i].equals(1.0f)) {

          int v = (int)(theOscMessage.get(0).floatValue() * 255);
          LightGroup l = (LightGroup)lightGroups.get(i);
          l.setColor2(v, l.g2, l.b2);
          l.sendMessage();


        }
      }

    } else if (p.equals("/green2")) { 

      for (int i = 0; i < activeAddr.length; i++) {
        if (activeAddr[i].equals(1.0f)) {

          int v = (int)(theOscMessage.get(0).floatValue() * 255);
          LightGroup l = (LightGroup)lightGroups.get(i);
          l.setColor2(l.r2, v, l.b2);
          l.sendMessage();


        }
      }

    } else if (p.equals("/blue2")) { 

      for (int i = 0; i < activeAddr.length; i++) {
        if (activeAddr[i].equals(1.0f)) {

          int v = (int)(theOscMessage.get(0).floatValue() * 255);
          LightGroup l = (LightGroup)lightGroups.get(i);
          l.setColor2(l.r2, l.g2, v);
          l.sendMessage();


        }
      }

    } else if (p.equals("/rate")) { 

      for (int i = 0; i < activeAddr.length; i++) {
        if (activeAddr[i].equals(1.0f)) {

          int v = (int)(theOscMessage.get(0).floatValue() * 127);
          LightGroup l = (LightGroup)lightGroups.get(i);
          l.setRate(v);
          l.sendMessage();

        }
      }

    } else if (p.equals("/brightness")) { 

      for (int i = 0; i < activeAddr.length; i++) {
        if (activeAddr[i].equals(1.0f)) {

          int v = (int)(theOscMessage.get(0).floatValue() * 127);

          println("brightness " + v);

          LightGroup l = (LightGroup)lightGroups.get(i);
          l.setBrightness(v);
          l.sendMessage();

        }
      }

    }
    
  }

}
interface OscReader {
  public void read(OscMessage theOscMessage);
}

OscReader reader = new LiveReader(); //new LemurReader();

public void oscEvent(OscMessage theOscMessage) {

  if (DEBUG) println("[OSC] " + theOscMessage.addrPattern());

  reader.read(theOscMessage);

}

public void oscMessage(String addr, float value) {
  OscMessage message = new OscMessage(addr);
  message.add(value);
  oscP5.send(message, myRemoteLocation);
}


public void setActiveAddr(Object[] activeAddr) {
  
  this.activeAddr = activeAddr;

}
  static public void main(String[] passedArgs) {
    String[] appletArgs = new String[] { "UFO_Commander" };
    if (passedArgs != null) {
      PApplet.main(concat(appletArgs, passedArgs));
    } else {
      PApplet.main(appletArgs);
    }
  }
}
