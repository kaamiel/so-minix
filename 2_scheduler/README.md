## sched

Celem zadania jest zaimplementowanie eksperymentalnego algorytmu szeregowania w systemie MINIX. Dotyczy on wyłącznie procesów użytkownika. Szeregowanie procesów systemowych pozostaje bez zmian.

Procesy użytkownika umieszczane są w trzech kolejkach związanych z priorytetami I, II i III. Początkowo każdy proces użytkownika otrzymuje priorytet I. Kiedy proces po raz 5-ty zużyje swój kwant czasu, zmienia priorytet na II. Gdy proces, pracując z priorytetem II, po raz 10-ty zużyje swój kwant czasu, zmienia priorytet na III. W celu uniknięcia zagłodzenia, procesy działające z priorytetem III, kiedy zużyją kolejne 20 kwantów czasu, zmieniają priorytet na I.

System wybiera do wykonania po jednym razie wszystkie procesy o priorytecie I, a następnie jeden proces o priorytecie II, po czym znowu wszystkie procesy o priorytecie I. Jeśli wszystkie procesy z kolejki II zostały wybrane do wykonania, system wybiera jeden proces z kolejki III.

### Przykład

```
I   p1 p2 p3
II  p4 p5
III p6 p7
```
Kolejność wykonywania procesów powinna być następująca:
```
p1 p2 p3 p4 p1 p2 p3 p5 p6 p1 p2 p3 p4 p1 p2 p3 p5 p7 p1 p2 p3 p4 …
```

### Implementacja

Implementacja powinna zawierać:

* konieczne zmiany w mikrojądrze oraz w serwerach `sched` i `pm`, przy czym za nagłówkiem każdej funkcji, która została dodana lub zmieniona, w tym samym wierszu, należy dodać komentarz `/* eas_2019 */`.

Dopuszczamy zmiany w katalogach:
* `/usr/src/minix/servers/sched`,
* `/usr/src/minix/servers/pm`,
* `/usr/src/minix/kernel`,
oraz w plikach nagłówkowych:
* `/usr/src/minix/include/minix/com.h`, który będzie kopiowany do `/usr/include/minix/com.h`,
* `/usr/src/minix/include/minix/config.h`, który będzie kopiowany do `/usr/include/minix/config.h`,
* `/usr/src/minix/include/minix/syslib.h`, który będzie kopiowany do `/usr/include/minix/syslib.h`.

### Wskazówki

* Do zmiany metody szeregowania można dodać nową funkcję systemową mikrojądra. Warto w tym przypadku wzorować się na przykład na funkcji `do_schedule()`.
* Przypominamy, że wstawianie do kolejki procesów gotowych wykonuje mikrojądro (`/usr/src/minix/kernel/proc.c`). Natomiast o przydzielaniu kwantu i priorytetu decyduje serwer `sched`.
* Nie trzeba pisać nowego serwera szeregującego. Można zmodyfikować domyślny serwer `sched`.
* Aby nowy algorytm szeregowania zaczął działać, należy wykonać `make; make install` w katalogu `/usr/src/minix/servers/sched` oraz w innych katalogach zawierających zmiany. Następnie trzeba zbudować nowy obraz jądra, czyli wykonać `make do-hdboot` w katalogu `/usr/src/releasetools` i zrestartować system. Gdyby obraz nie chciał się załadować lub wystąpił poważny błąd (`kernel panic`), należy przy starcie systemu wybrać opcję 6, która załaduje oryginalne jądro.

### Rozwiązanie

Poniżej przyjmujemy, że `ab123456` oznacza identyfikator studenta rozwiązującego zadanie. Należy przygotować łatkę (ang. patch) ze zmianami w sposób opisany w treści poprzedniego zadania.

Prosimy pamiętać o dodaniu odpowiednich komentarzy, ponieważ lista zmienionych funkcji uzyskana za pomocą polecenia `grep -r eas_2019 /usr/src` będzie miała wpływ na ocenę zadania. Wystarczy, że każda funkcja pojawi się na liście tylko raz, więc nie potrzeba umieszczać komentarzy w plikach nagłówkowych.

W repozytorium, w katalogu `studenci/ab123456/zadanie4` należy umieścić:
* patch ze zmianami,
* skrypt `kompiluj.sh`.

Uwaga: nie przyznajemy punktów za rozwiązanie, w którym łatka nie nakłada się poprawnie, które nie kompiluje się lub powoduje `kernel panic` podczas uruchamiania.
