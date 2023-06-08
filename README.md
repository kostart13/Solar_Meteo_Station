# Solar_Meteo_Station
Μετεωρολογικος σταθμός που λειτουργεί καθαρά με ηλιακή ενέργεια.

Δημιουργήσαμε έναν ηλιακό μετεωρολογικό σταθμό ο οποίος μετράει τη θερμοκρασία, την υγρασία, την ατμοσφαιρική πίεση, το υψόμετρο, την ταχύτητα και την κατευθυνση του ανέμου.
Πρόκειται για αναβάθμιση και ολοκλήρωση μιας προσπάθειας που ξεκινήσαμε πέρισυ. Κάναμε αρκετές βελτιώσεις και προσθήκες σε μια υπάρχουσα κατασκευή. 
Πέρα από την προφανή χρήση (της καταγραφής μετεορολογικών δεικτών) μπορεί να χρησιμοποιηθεί και σαν εργαλείο πρόβλεψης ακραίων καιρικών φαινομένων (π.χ. ισχυροί άνεμοι στην περιοχή μας που οδηγούν σε πλημμύρες και κλείσιμο δρόμων).

Λειτουργία:
Ο σταθμός μας αποτελείται από τέσσερα βασικά μέρη:

    esp8266 module με τους κατάλληλους αισθητήρες.
    Ανεμόμετρο και ανεμοδείκτη. (προστέθηκαν τώρα).
    Μπαταρία - φορτιστή μπαταρίας. (διορθώθηκαν κάποια προβλήματα φόρτισης)
    Ηλιακό πάνελ που φορτίζει την μπαταρία.

Ο esp8266 λειτουργεί σαν σημείο πρόσβασης (WiFi access point) και επιτρέπει σε οποιαδήποτε συσκευή να συνδεθεί μαζί του και να λάβει μετρήσεις από τους αισθητήρες (θερμοκρασία, υγρασία, ταχύτητα ανέμου, κατεύθυνση ανέμου, ατμοσφαιρική πίεση και υψόμετρο).
Αφού συνδεθεί κάποιος στέλνουμε στη συσκευή του τα στοιχεία σε με μορφή html χρησιμοποιώντας καλαίσθητα γραφικά.  

Τα σημαντικά στοιχεία της κατασκευής μας είναι:

    Πρόκειται για έναν κανονικό - λειτουργικό μετεωρολογικό σταθμό.
    Μπορεί οποιοσδήποτε να συνδεθεί και να δει μετεωρολογικές μετρήσεις.
    Οι μετρήσεις παρουσιάζονται σε γραφικό περιβάλλον στην οθόνη της συσκευής του.
    Δεν χρειάζεται σύνδεση στο διαδίκτυο, ο χρήστης συνδέεται απευθείας με τον ESP8266 που λειτουργεί σαν web server.(νέο χαρακτηριστικό)
    Χρησιμοποιούμε αδιάβροχο επαγγελματικό ηλεκτρολογικό κουτί (αντι για ένα απλό πλαστικό που είχαμε πριν).

Μελλοντικά θα μπορεί να στέλνει τις μετρήσεις στο cloud ή και να συνεργάζεται με σύστημα έγκαιρης προειδοποίησης έκτακτων φαινομένων. 
