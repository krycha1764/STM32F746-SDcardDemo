design assumptions:

command:

help
man jak nas poniesie
whatis jak nas bardzo poniesie

1. Nawigacja i przeglądanie struktury

    + pwd - Print the current directory - trzeba to ogarnąć na mikrokontrolerze żeby pamiętał gdzie aktualnie jesteśmy

    + ls – listuje zawartość katalogu. - infoirmacja do wyciągnięcia od karty, powinna podsyłąć info, można też dodać argumenty pozwalające na wyświetlanie:

        * Nazwa pliku/katalogu
        * Rozszerzenie
        * czy to katalog, czy plik ukryty, tylko do odczytu itp.
        * Czas i data ostatniej modyfikacji
        * Rozmiar pliku w bajtach (32-bitowa liczba)

    + cd (Change Directory) – zmienia bieżący katalog. - w górę musi pamiętać w dół bierze od karty

    + + **tree** - jak nam się będzie nudzić to można dodać ale to trzeba przebiec całą kartę praktycznie

2. Tworzenie i usuwanie

    + touch – tworzy nowy, pusty plik - to bym zmienił na coś innego bo mało użyteczne,

    + mkdir (Make Directory) – tworzy nowy katalog.- nie bawmy się w uprawnienia
    + rm (Remove) – usuwa pliki. - można dać jakieś modyfikatory podstawowe jak nam się będzi nudzić

3. Kopiowanie, przenoszenie i zmiana nazw

    + cp (Copy) – kopiuje pliki -nie bawmy się w kopiowanie rekurencyjnie
    + mv (Move) – przenosi pliki lub zmienia ich nazwę.

4. Podgląd i edycja zawartości plików

    + cat (Concatenate) – wyświetla całą zawartość pliku w terminalu.
    + head – wyświetla pierwsze 10 linii pliku (head -n 5 pokaże pierwsze 5 linii).
    + tail – wyświetla ostatnie 10 linii pliku.
    + du (Disk Usage) – pokazuje, ile miejsca na dysku zajmują poszczególne pliki, można dodac modyfikatory na przeliczanie rozmiaru


7. dodatkowe można zrobić w wolnej chwili i chęci

    + file – rozpoznaje typ pliku (nie patrzy na rozszerzenie, ale na zawartość nagłówka).
    + stat – wyświetla bardzo szczegółowe informacje o pliku (rozmiar, uprawnienia, dokładne daty odyfikacji i dostępu).
    