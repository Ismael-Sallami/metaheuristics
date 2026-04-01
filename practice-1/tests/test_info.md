Con placer. Aquí tienes una explicación de cada test agrupada por bloque:

---

## BLOQUE 1 — Constructor y dimensiones

**1.1** Instancia `PortfolioProblem` con datos de entrenamiento (2015-2024) y verifica que no lanza ninguna excepción. Comprueba que el CSV se lee correctamente.

**1.2** Igual pero con el flag `is_2025=true`, es decir, filtrando solo el año 2025. Verifica que ese subconjunto de datos también se carga bien.

**1.3** Llama a `getSolutionSize()` y comprueba que devuelve exactamente el número de empresas esperado (30, 97 o 457 según el mercado).

**1.4** Llama a `getSolutionDomainRange()` y verifica que devuelve exactamente el par `(lo, hi)` con el que se construyó el problema.

**1.5** Intenta construir el problema con un fichero que no existe y verifica que se lanza una excepción. Sin esto, un error de ruta pasaría desapercibido.

---

## BLOQUE 2 — isValid

**2.1** Construye manualmente una solución asignando `hi` a tantas empresas como caben hasta sumar 1. Verifica que `isValid` la acepta.

**2.2** Pone un peso negativo (`-0.001`) y verifica que `isValid` lo rechaza. Los pesos negativos son matemáticamente absurdos en un portfolio.

**2.3** Pone un peso de `hi + 0.01`, superando el límite máximo legal, y verifica que `isValid` lo detecta.

**2.4** Pone un peso de `lo - 0.001`: positivo pero por debajo del mínimo permitido (micro-inversión ilegal). Verifica que `isValid` lo rechaza.

**2.5** Asigna `hi` a tres empresas, de forma que la suma supera 1.0, y verifica que `isValid` lo rechaza.

**2.6** Pone solo `lo` en una empresa (suma muy inferior a 1) y verifica que `isValid` lo rechaza.

**2.7** Toma una solución válida generada por `createSolution` y le añade `1e-10` a un peso activo. Verifica que `isValid` lo sigue aceptando, porque la tolerancia es `1e-8` y esa impureza de coma flotante no debería romper nada.

**2.8** Vector de todos ceros: la suma es 0, no 1. Verifica que `isValid` lo rechaza.

---

## BLOQUE 3 — createSolution

**3.1** Genera una solución aleatoria y comprueba que pasa `isValid`. Caso básico de sanidad.

**3.2** Genera 100 soluciones seguidas y verifica que todas son válidas. Detecta problemas de robustez estadística o seeds incorrectas.

**3.3** Verifica que el vector devuelto por `createSolution` tiene exactamente `n` elementos.

**3.4** Sobre 20 soluciones generadas, comprueba que al menos una tiene algún peso a cero. El enunciado exige que las soluciones sean esparsas (entre 1 y N-1 ceros).

**3.5** Sobre 50 soluciones, verifica que ningún peso activo (mayor que 0) está por debajo de `lo`. Detectaría que `fix` no está eliminando micro-inversiones correctamente.

---

## BLOQUE 4 — fix

**4.1** Llama a `fix` con un vector de todos ceros (suma = 0, división por cero posible). Verifica que la función sobrevive y produce una solución válida.

**4.2** Pone `sol[0]=5.0` y `sol[1]=-2.0`, valores completamente ilegales. Verifica que `fix` los repara y devuelve algo válido.

**4.3** Aplica `fix` a una solución ya válida y verifica que sigue siendo válida después. `fix` no debe romper lo que ya está bien.

**4.4** Vector con un solo peso de `0.5` (suma = 0.5, no normalizada). Verifica que `fix` la normaliza correctamente hasta llegar a suma = 1.

**4.5** Todos los pesos a `hi`: la suma es `n * hi`, enormemente mayor que 1. Verifica que `fix` la reduce correctamente.

**4.6** Todos los pesos a `lo/2`: son positivos pero ilegales (micro-inversiones). Verifica que `fix` los pone a cero y redistribuye el presupuesto.

**4.7** Llama a `fix` con `sol[0]=100.0` y verifica que ningún elemento del resultado es `NaN` ni `Inf`. Detecta divisiones por cero silenciosas.

**4.8** Aplica `fix` dos veces a la misma solución rota y verifica que ambas pasadas dan exactamente el mismo resultado. Una función de reparación debe ser idempotente.

---

## BLOQUE 5 — fitness

**5.1** Verifica que `fitness` no devuelve `NaN` para una solución válida. El caso más básico de sanidad del evaluador.

**5.2** Verifica que `fitness` no devuelve `Inf`. Detectaría divisiones por cero o desbordamientos numéricos.

**5.3** Evalúa la misma solución dos veces y verifica que el resultado es idéntico. `fitness` debe ser puramente determinista (no depende de estado aleatorio).

**5.4** Construye una solución rota, la repara con `fix`, y verifica que `fitness` sobre ella no es `NaN`. Prueba la cadena completa reparación → evaluación.

**5.5** Genera dos soluciones distintas, calcula su fitness y verifica que ambas llamadas terminan sin explotar. No hace un assert duro sobre si son distintas (podría coincidir por azar).

**5.6** Verifica que `getBeneficio` no devuelve `NaN` para una solución válida.

**5.7** Comprueba que `fitness(sol) ≤ getBeneficio(sol)`. Como `fitness = beneficio - λ·riesgo` con `λ=500 > 0` y la matriz de covarianza es semidefinida positiva (riesgo ≥ 0), esto debe cumplirse siempre. Si fallara, indicaría un bug en el cálculo del riesgo.

**5.8** Toma la misma solución y la evalúa con el problema de entrenamiento y con el de test (2025). Verifica que ninguno de los dos devuelve `NaN`. Prueba que los dos objetos coexisten sin interferirse.

---

## BLOQUE 6 — getGreedyHeuristic

**6.1** Llama a `getGreedyHeuristic(i)` para todas las empresas y verifica que ninguna devuelve `NaN`.

**6.2** Igual, verificando que ninguna devuelve `Inf`.

**6.3** Llama dos veces a `getGreedyHeuristic` para las primeras 5 empresas y verifica que el resultado es idéntico. La heurística no depende de estado aleatorio, debe ser constante.

**6.4** Verifica que no todas las empresas tienen la misma heurística. Si todas fueran iguales, el greedy no tendría información para ordenar y habría un bug en el cálculo.

---

## BLOQUE 7 — Integración end-to-end

**7.1** Bucle de 50 iteraciones: `createSolution → fitness → isValid`. Verifica que las tres funciones encadenan correctamente sin errores ni resultados NaN.

**7.2** Genera 20 soluciones válidas, aplica `fix` a cada una, y verifica que siguen siendo válidas. Confirma que `fix` no introduce regresiones.

**7.3** Genera 100 soluciones, guarda el mejor y el peor fitness. Verifica que el mejor es estrictamente mayor que el peor, confirmando que hay variedad real en el espacio de búsqueda.

**7.4** Simula manualmente el algoritmo greedy: obtiene las heurísticas, las ordena, y distribuye el presupuesto. Verifica que el fitness de la solución resultante no es NaN.

**7.5** Toma una solución válida y aplica el movimiento de la búsqueda local (transfiere el 40% del peso de empresa `i` a empresa `j`). Verifica que la suma total de pesos se conserva exactamente, ya que el movimiento no debe crear ni destruir presupuesto.

**7.6** Genera 500 soluciones consecutivas sin verificar nada más que que no hay crash ni fuga de memoria. Prueba de estrés puro.

---

## BLOQUE 8 — Edge cases numéricos

**8.1** Construye una solución donde la última empresa tiene exactamente `lo` (el mínimo legal). Verifica que `isValid` lo acepta, es decir, que el límite inferior es inclusivo.

**8.2** Si `1/hi` es un entero exacto (como en S&P 100 con `hi=0.05`: 20 empresas × 0.05 = 1.0), construye esa solución y verifica que es válida. Comprueba que el límite superior también es inclusivo.

**8.3** Añade `5e-9` a un peso activo de una solución válida (quedando la suma en `1.0 + 5e-9`). Solo verifica que no hay crash; el resultado depende de si `5e-9 < 1e-8` (la tolerancia).

**8.4** Solo aplica si `hi ≥ 1.0` (no es el caso en ninguno de los tres mercados). Construiría una solución con todo el presupuesto en una empresa y verificaría que `fitness` no es NaN.

**8.5** Crea dos instancias independientes (train y test), modifica agresivamente la primera, y verifica que la segunda sigue funcionando correctamente. Detectaría estado global compartido entre instancias.

**8.6** Vector de tamaño `n` con todos los pesos a `-1.0` (suma muy negativa, todos ilegales). Verifica que `fix` los convierte en algo válido partiendo de negativos puros.