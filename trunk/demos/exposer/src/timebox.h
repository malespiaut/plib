
void vcr_fastReverse ( puObject * ) ;
void vcr_reverse     ( puObject * ) ;
void vcr_stop        ( puObject * ) ;
void vcr_play        ( puObject * ) ;
void vcr_fastPlay    ( puObject * ) ;
void vcr_groundSpeed ( puObject *me ) ;


void reverseRegionCB (puObject *) ;
void deleteAll () ;
void deleteRegionCB (puObject *) ;


void zoom_nrm_CB ( puObject * ) ;
void zoom_in_CB  ( puObject * ) ;
void zoom_out_CB ( puObject * ) ;


void add_1_CB ( puObject * ) ;
void add_2_CB ( puObject * ) ;
void add_5_CB ( puObject * ) ;


void deleteRegionAndCompressCB ( puObject *me ) ;
void deleteEventCB (puObject *) ;
void addNewEventCB (puObject *) ;
void timescrollerCB ( puObject *ob ) ;
void updateVCR () ;
void drawTimeBox () ;

void updateEventQueue ( int button, int x, int y, int new_click ) ;

float getCursor () ;
float getMaxTime () ;
void  setMaxTime ( float t ) ;
float getVCRGroundSpeed () ;
void  setVCRGroundSpeed ( float t ) ;
float getVCRGroundPosition () ;
void  setVCRGroundPosition ( float t ) ;

void initTimeBox () ;

