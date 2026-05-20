# Informe de contraste entre teoría y código

Este documento resume por qué la implementación del proyecto coincide con la teoría de la práctica en los puntos más relevantes para la búsqueda local suave y su uso dentro de los algoritmos evolutivos y meméticos.

## 1. Búsqueda local suave: teoría y código coinciden

La teoría del material de apoyo describe la búsqueda local suave como una variante de la BL de la práctica 1, pero menos agresiva: mueve un 15% del valor de un gen a otro, usa un vecindario de pares ordenados `(i, j)` y limita la exploración a 100 evaluaciones por llamada. Eso se explica explícitamente en [bases_teoricas-NOTEBOOKLM.md](../teoria-recursosExtra/bases_teoricas-NOTEBOOKLM.md).

La implementación sigue exactamente ese esquema en [soft_local_search.cpp](../code/src/soft_local_search.cpp) y su interfaz en [soft_local_search.h](../code/inc/soft_local_search.h):

- Se construye el vecindario con todos los pares `i != j`.
- Se baraja una vez al iniciar la exploración.
- Para cada par se calcula `transfer = solution[i] * ratio` con `ratio = 0.15`.
- Se crea un vecino moviendo masa de `i` a `j`.
- Se valida que ambos valores sigan dentro de los límites del problema.
- Solo si el vecino mejora el fitness, se acepta la nueva solución.
- El proceso se corta cuando se agotan las 100 evaluaciones locales o cuando ya no quedan movimientos útiles en la pasada actual.

Por eso el pseudocódigo del informe debe hablar de "barajar una vez" y no de "volver a barajar" en cada mejora. El código no rehace la permutación completa tras una mejora; lo que hace es mantener el orden ya barajado y reiniciar el contador interno de exploración cuando encuentra una solución mejor.

## 2. Por qué hay dos funciones de BL suave

En el código hay dos niveles de uso para la BL suave, y eso tiene un sentido claro:

- La sobrecarga sin estado de [soft_local_search.cpp](../code/src/soft_local_search.cpp) crea un `SoftLSState` local temporal y llama a la versión con estado.
- La sobrecarga con estado reutiliza un `SoftLSState` persistente para continuar la exploración desde donde se quedó.

Ese estado persistente está definido en [soft_local_search.h](../code/inc/soft_local_search.h) como un conjunto de tres piezas: el vecindario ya generado, el cursor del siguiente movimiento y la dimensión de la solución. La función auxiliar `ensure_soft_ls_state` en [soft_local_search.cpp](../code/src/soft_local_search.cpp) garantiza que el vecindario se reconstruya solo cuando cambia la dimensión o cuando el estado está vacío.

El sentido de esta separación es práctico:

- Cuando se quiere una BL normal, basta con la versión sin estado.
- Cuando se quiere una BL encadenada, como en MA-LSCh, conviene conservar el estado para no reiniciar desde cero en cada aplicación.

Eso encaja con la teoría del memético adaptativo: el refinamiento local no arranca siempre desde el mismo punto, sino que puede continuar una búsqueda previa si el individuo sobrevive en la población. Esa idea se usa en [memetic_lsch.cpp](../code/src/memetic_lsch.cpp).

## 3. Coincidencia con los algoritmos poblacionales

La parte poblacional también encaja con el guion teórico.

En [genetic_algorithm.cpp](../code/src/genetic_algorithm.cpp), el AGG y el AGE usan la misma infraestructura común de [ea_common.h](../code/inc/ea_common.h):

- selección por torneo de tamaño 3;
- cruce aritmético o BLX;
- mutación por transferencia con reparación posterior;
- evaluación con reparación previa mediante `problem.fix()`;
- comparación siempre en modo de maximización, ya que `ea_better(a, b)` devuelve `a > b`.

La lógica del AGG coincide con el esquema generacional elitista: se crea una población de hijos, se evalúa completa y, si el mejor padre supera al mejor hijo, se conserva elitismo reemplazando al peor hijo. La del AGE también coincide: se generan dos hijos por iteración, se evalúan y se insertan en la población solo si superan a los peores.

## 4. Coincidencia de los meméticos con la BL suave

En [memetic_algorithm.cpp](../code/src/memetic_algorithm.cpp) se ve que la BL suave se usa como etapa de intensificación periódica. Eso coincide con el guion: cada 10 generaciones se aplica sobre un subconjunto de la población, según la variante elegida.

La versión con estado de [memetic_lsch.cpp](../code/src/memetic_lsch.cpp) tiene además un matiz importante:

- guarda `SoftLSState` por individuo;
- reinicia el estado en descendientes generados por cruce o mutación;
- preserva el estado del elite cuando sobrevive;
- reutiliza el estado al volver a aplicar la BL al mismo individuo.

Ese diseño explica por qué el operador se divide en `ensure_soft_ls_state`, `reset_soft_local_search_state` y las dos sobrecargas de `apply_soft_local_search`: no es duplicación accidental, sino separación entre inicialización, reinicio y reutilización de la búsqueda local.

## 5. Conclusión

La implementación sí coincide con la teoría en lo esencial:

- la BL suave es primer mejor, con 15% y 100 evaluaciones;
- el orden del vecindario se baraja una sola vez por fase de exploración;
- el memético la usa como intensificación periódica;
- la variante LSCh añade memoria para continuar la BL entre llamadas.

La única corrección importante frente al texto inicial del informe era precisamente esa: no decir que el vecindario se vuelve a barajar cada vez que mejora una solución, porque el código real no hace eso. En su lugar, la mejora reinicia solo la exploración interna y mantiene el orden barajado ya construido.