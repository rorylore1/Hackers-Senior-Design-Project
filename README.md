Senior Design Project at The University of Texas at Dallas  

ABSTRACT  

Our project, named Hackers, is a puzzle game designed to introduce engineering concepts, through collaboration and communication, to students ranging from 6th to 12th grade. It consists of two main components: the Game Box and the Game Manual.  

The Game Box is a briefcase-style container holding various modules based on engineering concepts such as binary conversion, engineering equations, wave functions, and physical components like resistors and oscilloscopes. These modules must all be solved under a given time limit, and each module is given a random selection of preset configurations each time the game is started.  

The Manual is a document given to the player separately from the Game Box. It contains information about the modules. Each module has aspects of the puzzle omitted that require detailed communication between the player with the Game Box and the player with the Manual to solve.  

By presenting puzzles that mimic the logic behind engineering concepts, Hackers can provide an entertaining challenge of cooperation between peers, and a powerful introduction to some of the components and ideas that most engineers rely on in their respective fields.  

PROJECT DEVELOPMENT  

The development was split into three stages.  

1. Virtual proof of concept via Tinkercad.  
2. Software implementation and hardware proof of design via breadboards.  
3. Software and hardware final implementation via PCBs.  

PROJECT IMPLEMENTATION  

The project is a union of six different modules, each connected to the power system. The resistor, logic gates, and binary conversion modules are using esp32s as their main microcontroller, while the hidden maze, oscilloscope, and equation modules are using arduino minis. The power system is connected to any wall outlet.  

In our final prototype, all modules except the binary conversion module and the master module were implemented on a PCB board and connected to the power supply. These modules were available in free play mode, where it would randomize the problem and when solved correctly, it would generate a new problem.  

COMPONENTS  

1. The Game Box  
a. A 3D printed box with the power supply in the bottom compartment, then the 6 modules, then the master module on the lid.  
2. The Manual  
a. A stack of papers stabled together that has instructions on how to play, solutions to each module, and applications of the concepts.  
3. The Power System  
a. A custom PCB to source electrical power from a wall outlet for use by the game box and storage in a battery.  
4. Communication Between Modules  
a. Using I2C communication via custom SDA and SCL lines with the Master module as the master and each Puzzle module as slaves.  
5. Master Control Module  
a. Displays the game difficulty level, the number of lives left, and the time left.  
6. Logic Gates Puzzle  
a. Players must use logic gates to solve a Boolean expression by arranging gates to match the provided equation.  
7. Oscilloscope Puzzle  
a. Players identify properties of a periodic signal, such as amplitude, frequency, and vertical shift. Then they adjust dials to match the correct values.  
8. Base Conversion Puzzle  
a. Players convert numbers between bases (e.g., binary, decimal, and hexadecimal) and solve arithmetic problems in different bases.  
9. Equations Puzzle
a. Players identify fundamental engineering equations by inputting values corresponding to displayed prompts.  
11. Resistor Puzzle  
a. Players identify the 4-band color code corresponding to a given resistance and tolerance.  
12. Hidden Maze Puzzle  
a. Players navigate a maze using verbal communication to guide movement.  
