idf_component_register(
    SRCS "DateAgenda.c" "DateAlarm.c" "DateDB.c" "DateDay.c" "DateDisplay.c" "DateGlobals.c" "DateMonth.c" "DatePref.c" "DateTransfer.c" "DateWeek.c" "Datebook.c" "ToDoDB.c"
    REQUIRES "driver" "libpumpkin" "libpit" "fatfs" "esp_timer" "esp_wifi" "libmath"
)
