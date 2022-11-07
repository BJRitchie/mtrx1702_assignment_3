process:

1. Check and read binary file into array of structs with:
    a. terrain 
    b. height 
    c. science goal (y/n)
    d. rover location (y/n)
    c. corrupt (y/n) - parity check

2. Clean corrupt data 
    - most common surrounding measurement
    - store location of science goal and rover (if not corrupt)

3. Produce topography and terrain arrays

4. check for type:
    type m:
    1. check inputs
    2. locate relevant data and print

    type c:
    1. read/check inputs (until "end" input)
    2. make movement iterably
        - check for bounds and impossible movements
        - iterably calculate the energy usage
            - dependent on: slope, terrain
        - if all movements are possible on "end" input, the path is feasible

    type f:
    1. read/check inputs
    2. conduct a depth-first search
        - start from rover position
        - check all accessible (slope < 1.5, terrain not type 3) locations for the science goal
            - an impossible move is not added to the "goto" list
        - if its not found, the path is not feasible
