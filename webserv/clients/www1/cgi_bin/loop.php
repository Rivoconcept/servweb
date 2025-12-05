<?php
// script qui boucle mais se protège avec set_time_limit pour ne pas bloquer indéfiniment
set_time_limit(5);
header('Content-Type: text/plain');
echo "Start loop...\n";
$start = time();
while (true) {
    // sleep to avoid busy loop
    sleep(1);
    echo "tick\n";
    flush();
    if (time() - $start > 4) break; // safety: break after ~4s
}
echo "Done\n";
?>
