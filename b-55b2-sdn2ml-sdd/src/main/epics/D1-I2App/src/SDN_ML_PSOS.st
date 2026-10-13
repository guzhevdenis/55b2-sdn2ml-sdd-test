program SDN_ML_PSOS

short SDN_SetState;         assign SDN_SetState         to "{UNIT}-SDN-StateMachine-setState";  monitor SDN_SetState;
short SDN_GetState;         assign SDN_GetState         to "{UNIT}-SDN-StateMachine-getState";

short DAQ_SetState;         assign DAQ_SetState         to "{UNIT}-DAQ-StateMachine-setState";  monitor DAQ_SetState;
short DAQ_GetState;         assign DAQ_GetState         to "{UNIT}-DAQ-StateMachine-getState";

short SDN_MasterSetState;   assign SDN_MasterSetState   to "D1-I2-SDN-StateMachine-setState";   monitor SDN_MasterSetState;
short SDN_MasterGetState;   assign SDN_MasterGetState   to "D1-I2-SDN-StateMachine-getState";   monitor SDN_MasterGetState;

short DAQ_MasterSetState;   assign DAQ_MasterSetState   to "D1-I2-DAQ-StateMachine-setState";   monitor DAQ_MasterSetState;
short DAQ_MasterGetState;   assign DAQ_MasterGetState   to "D1-I2-DAQ-StateMachine-getState";   monitor DAQ_MasterGetState;

short SDN_PrevState; //variables for tracking previous states
short DAQ_PrevState;

ss STATE_SWITCH {
    state SYNCRONYZE {
    	entry {
    		SDN_SetState = 1;
    		DAQ_SetState = 1;
            SDN_MasterSetState = 1;
            DAQ_MasterSetState = 1;
    	}
        when(pvPutComplete(SDN_SetState) && pvPutComplete(DAQ_SetState) &&
             pvPutComplete(SDN_MasterSetState) && pvPutComplete(DAQ_MasterSetState)) {
                SDN_PrevState = 1;
                DAQ_PrevState = 1;
        } state SELECT_STATE
    }
    state SELECT_STATE {
        when(SDN_MasterSetState == 1 && SDN_PrevState != 1) { } state SDN_OFF
        when(SDN_MasterSetState == 4 && SDN_PrevState != 4) { } state SDN_ON
        when(SDN_MasterSetState == 7 && SDN_PrevState != 7) { } state SDN_RUNNING
        when(DAQ_MasterSetState == 1 && DAQ_PrevState != 1) { printf("DAQ OFF"); } state DAQ_OFF
        when(DAQ_MasterSetState == 4 && DAQ_PrevState != 4) { printf("DAQ ON" ); } state DAQ_ON
        when(DAQ_MasterSetState == 7 && DAQ_PrevState != 7) { printf("DAQ RUN"); } state DAQ_RUNNING
    }
    state SDN_OFF {
        when(SDN_PrevState == 4 && SDN_SetState == 4) {
            SDN_SetState = 1;
            SDN_PrevState = 1;
            pvPut(SDN_SetState);
        } state SELECT_STATE
        when(SDN_PrevState != 4 || SDN_SetState != 4) { //when transition from RUNNING to OFF or not all STMs in the same states
            SDN_SetState = 4;
            pvPut(SDN_SetState); //Set ON states
            SDN_SetState = 1;
            pvPut(SDN_SetState); //Set OFF states
            SDN_PrevState = 1;
        } state SELECT_STATE
    }
    state SDN_ON {
        when(SDN_PrevState == 1 || SDN_PrevState == 7 ) {
            SDN_SetState = 4;
            SDN_PrevState = 4;
            pvPut(SDN_SetState);
        } state SELECT_STATE
    }
    state SDN_RUNNING {
        when(SDN_PrevState == 4 && SDN_SetState == 4) {
            SDN_SetState = 7;
            SDN_PrevState = 7;
            pvPut(SDN_SetState);
        } state SELECT_STATE
        when(SDN_PrevState != 4 || SDN_SetState != 4) { //when transition from OFF to RUNNING or not all STMs in the same states
            SDN_SetState = 4;
            pvPut(SDN_SetState); //Set ON states
            SDN_SetState = 7;
            pvPut(SDN_SetState); //Set RUNNING states
            SDN_PrevState = 7;
        } state SELECT_STATE
    }
    state DAQ_OFF {
        when(DAQ_PrevState == 4 && DAQ_SetState == 4) {
            DAQ_SetState = 1;
            DAQ_PrevState = 1;
            pvPut(DAQ_SetState);
        } state SELECT_STATE
        when(DAQ_PrevState != 4 || DAQ_SetState != 4) { //when transition from RUNNING to OFF or not all STMs in the same states
            DAQ_SetState = 4;
            pvPut(DAQ_SetState); //Set ON states
            DAQ_SetState = 1;
            pvPut(DAQ_SetState); //Set OFF states
            DAQ_PrevState = 1;
        } state SELECT_STATE
    }
    state DAQ_ON {
        when(DAQ_PrevState == 1 || DAQ_PrevState == 7) {
            DAQ_SetState = 4;
            DAQ_PrevState = 4;
            pvPut(DAQ_SetState);
        } state SELECT_STATE
    }
    state DAQ_RUNNING {
        when(DAQ_PrevState == 4 && DAQ_SetState == 4) {
            DAQ_SetState = 7;
            DAQ_PrevState = 7;
            pvPut(DAQ_SetState);
        } state SELECT_STATE
        when(DAQ_PrevState != 4 || DAQ_SetState != 4) { //when transition from OFF to RUNNING or not all STMs in the same states
            pvPut(DAQ_SetState); //Set ON states
            DAQ_SetState = 7;
            pvPut(DAQ_SetState); //Set RUNNING states
            DAQ_PrevState = 7;
        } state SELECT_STATE
    }
}