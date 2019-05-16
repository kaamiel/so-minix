## distort time

### Wprowadzenie

Jak wskazują badania naukowe i nasze własne doświadczenia, percepcja czasu zmienia się wraz z wiekiem
([https://en.wikipedia.org/wiki/Time_perception#Changes_with_age](https://en.wikipedia.org/wiki/Time_perception#Changes_with_age)).
Im jesteśmy starsi, tym szybciej zdaje się płynąć czas. Można podejrzewać, że to zjawisko jest jednym
z powodów, dla których międzypokoleniowy dialog bywa niezwykle trudny – gdyby tylko starsze pokolenie
mogło przekonać swoje dzieci i wnuki, że należy „chwytać dzień”, bo czas płynie tak szybko, a sami zrozumieli,
że minuty mogą dłużyć się niczym godziny, gdy odliczamy czas do weekendu...

### Zadanie

Naszym zadaniem jest umożliwienie procesom MINIX-a uproszczoną manipulację upływem czasu innych procesów
(potomków i przodków w linii prostej). W tym celu należy m.in. zaimplementować wywołanie systemowe
`PM_DISTORT_TIME`, funkcję biblioteczną `int distort_time(pid_t pid, uint8_t scale)` (dostępną przez `unistd.h`)
oraz dokonać modyfikacji istniejących wywołań systemowych: `PM_GETTIMEOFDAY` i `PM_CLOCK_SETTIME`.

Efekt wywołania dodanej funkcji systemowej powinien być następujący:
* Jeśli proces wywołujący, C, jest przodkiem procesu P o podanym numerze `pid`, upływ czasu odczytywanego przez P
  za pomocą `PM_GETTIMEOFDAY` jest przyspieszany (skalowany) zgodnie z wartością `scale`.
* Jeśli proces wywołujący, C, jest potomkiem procesu P o podanym numerze pid, upływ czasu odczytywanego przez P
  za pomocą `PM_GETTIMEOFDAY` jest opóźniany (skalowany) zgodnie z wartością `scale`.
* Ponowne wywołanie funkcji `distort_time` dla tego samego procesu C, jeśli zakończy się sukcesem, powinno nadpisać
  poprzednio ustalony współczynnik przeskalowania upływu czasu.

W przypadku pomyślnego przeskalowania czasu, funkcja `distort_time` powinna zwrócić kod `OK` (wartość 0). Wszelkie
błędy występujące na drodze do przeskalowania czasu powinny skutkować zwróceniem odpowiedniej wartości innej
niż `OK` (niektóre z takich sytuacji zostały opisane w punkcie Przypadki szczególne).

Jak wspomniano, proces P mierzy upływ czasu za pomocą istniejącego wywołania systemowego `PM_GETTIMEOFDAY`.
Gdy upływ czasu procesu P zostanie przeskalowany o wartość różną od 1, jedynie pierwszy odczyt `PM_GETTIMEOFDAY`
będzie zgodny z rzeczywistością i stanie się punktem odniesienia w czasie. Każdy kolejny odczyt zwróci
przeskalowaną wartość względem ustalonego punktu odniesienia. Na przykład, jeśli przyspieszenie upływu czasu
jest dwukrotne, P odczyta najpierw prawdziwy czas, `X`, za pomocą `PM_GETTIMEOFDAY`, a po 5 sekundach odczyt przy
użyciu `PM_GETTIMEOFDAY` wskaże czas (`X` + 10 sekund). Zgodnie z obecną implementacją `PM_GETTIMEOFDAY`, czas po
przeskalowaniu powinien być zwracany z dokładnością do mikrosekund.

Wywołanie funkcji systemowej `PM_CLOCK_SETTIME` przez którykolwiek z działających procesów powinno zresetować
punkty odniesienia w czasie dla wszystkich procesów, tzn.: `PM_GETTIMEOFDAY` powinno ponownie zwrócić
procesowi P rzeczywisty czas (nowy punkt odniesienia) i dopiero kolejne odczyty zostaną przeskalowane.

### Przypadki szczególne

* Jeśli proces o podanym `pid` nie istnieje: `return EINVAL`.
* Jeśli `pid` procesu C i podany `pid` procesu P są takie same: `return EPERM`.
* Jeśli procesy P i C nie są spokrewnione w linii prostej: `return EPERM`.
* Jeśli `scale == 0`: odczytywany czas dla procesu P powinien stać w miejscu.

###Przykład

```
P1: proces
P2: proces powstały w wyniku wywołania fork() przez P1
P3: proces-obserwator

/* Procesy P1, P2 zakłócają sobie wzajemnie poczucie upływu czasu. */
P1.distort_time(P2, 3)
P2.distort_time(P1, 2)

/* Procesy odczytują aktualny czas - odczytane wartości są prawdziwe, podobne do wartości odczytanej przez P3. */
T1_start = P1.gettimeofday()
T2_start = P2.gettimeofday()
T3_start = P3.gettimeofday()

/* Procesy zasypiają na 5 sekund czasu rzeczywistego - działanie funkcji sleep nie podlega zaburzeniom czasu. */
P1.sleep(5)
P2.sleep(5)
P3.sleep(5)

/* Procesy odczytują ponownie czas. */
T1_end = P1.gettimeofday()
T2_end = P2.gettimeofday()
T3_end = P3.gettimeofday()

/* Różnice podane są w przybliżeniu. */
T1_end - T1_start = 2.5 s
T2_end - T2_start = 15 s
T3_end - T3_start = 5 s

/* Jeden z procesów ustala nowy czas. */
P3.settimeofday(T3_end)
T1_start2 = P1.gettimeofday()
assert(T1_start2 prawie równy T3_end)
P1.sleep(3)
T1_end2 = P1.gettimeofday()
assert((T1_end2 - T1_start2) prawie równe 1.5 s)

/* P2 nadpisuje ustalone wcześniej przeskalowanie dla P1. */
P2.distort_time(P1, 4)
T1_start3 = P1.gettimeofday()
P1.sleep(10)
T1_end3 = P1.gettimeofday()
assert((T1_end3 - T1_start3) prawie równe 2.5 s)
```

### Postać rozwiązania

Należy przygotować łatkę (ang. patch) ze zmianami w katalogu `/usr`. Plik zawierający łatkę o nazwie
`ab123456.patch` uzyskujemy za pomocą polecenia
```bash
diff -rupN oryginalne-źródła/usr/ moje-rozwiązanie/usr/ > ab123456.patch
```
gdzie `oryginalne-źródła` to ścieżka do niezmienionych źródeł MINIX-a, natomiast `moje-rozwiązanie` to ścieżka do
źródeł MINIX-a zawierających rozwiązanie. Tak użyte polecenie `diff` rekurencyjnie przeskanuje pliki ze ścieżki
`oryginalne-źródła/usr`, porówna je z plikami ze ścieżki `moje-rozwiązanie/usr` i wygeneruje plik `ab123456.patch`,
który podsumowuje różnice. Tego pliku będziemy używać, aby automatycznie nanieść zmiany na nową kopię MINIX-a,
gdzie będą przeprowadzane testy rozwiązania.

Umieszczenie łatki w katalogu / nowej kopii MINIX-a i wykonanie polecenia
```bash
patch -p1 < ab123456.patch
```
powinno skutkować naniesieniem wszystkich oczekiwanych zmian wymaganych przez rozwiązanie. Należy zadbać,
aby łatka zawierała tylko niezbędne różnice.

Po naniesieniu łatki domyślnie zostaną wykonane polecenia:
* `make && make install` w katalogach `/usr/src/minix/servers/pm` oraz `/usr/src/lib/libc`,
* `make do-hdboot` w katalogu `/usr/src/releasetools`,
* `reboot`.

Jeżeli rozwiązanie wymaga podjęcia innych kroków instalacji, autor jest proszony o dołączenie pliku `setup.sh`,
który przeprowadzi instalację rozwiązania po uruchomieniu z katalogu `/`. Ostatnim poleceniem w pliku `setup.sh`
powinno być polecenie `reboot`.
