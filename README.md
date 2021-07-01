# Program linuxowy obrazujący dostępu do wspólnych zasobów w systemie operacyjnym

## Opis:
Program działa w wersji tekstowej w terminalu. Pokazuje dostęp wątków do 1 zasobu, jakim jest most. Każdy wątek chce przejechać do most, by dojechać do miasta.
Wątki krążą od miasta A do B i odwrotnie. Zasobem współdzielonym jest most, który jest zarządzany przez arbitra. Arbiter korzysta z kolejki FIFO i przydziela
dostęp do mostu temu wątkowi, który pierwszy ustawił się w kolejce, niezależnie z której strony.

## Sposób użycia:
Wariant A:
./main <liczba_wątków>
Wariant B:
./condBridge <liczba_wątków>

## Wysłanie sygnału:
CTRL + C - następuje wtedy bezpieczne zakończenie działania programu, oznacza to, że pamięć
przydzielona zostaje zwolniona a wszystkie wątki i mutexy zostają przerwane i/lub zakończone.

## Przykłady użycia:
- ./condBridge 5(wariant B)
- ./main 5(wariant A)
