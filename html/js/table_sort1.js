function TABLESORT(grid) {
var i = 0;
//var grid = document.getElementById('resultTable');

//alert(i);

    grid.onclick = function(e) {
    
    alert(i);
    
      if (e.target.tagName != 'TH') return;

      // ≈сли TH -- сортируем
      sortGrid(e.target.cellIndex, e.target.getAttribute('data-type'));
    };

    function sortGrid(colNum, type) {
      var tbody = grid.getElementsByTagName('tbody')[0];

      // —оставить массив из TR
      var rowsArray = [].slice.call(tbody.rows);

      // определить функцию сравнени€, в зависимости от типа
      var compare;

      switch (type) {
        case 'number':
          if (i == 0) {
      		compare = function(rowA, rowB) {
        		return rowA.cells[colNum].innerHTML - rowB.cells[colNum].innerHTML;
		    };
          }
          else {
	         compare = function(rowA, rowB) {
	            return rowB.cells[colNum].innerHTML - rowA.cells[colNum].innerHTML;
	         };
          }
          break;
        case 'string':
          compare = function(rowA, rowB) {
            return rowA.cells[colNum].innerHTML > rowB.cells[colNum].innerHTML ? 1 : -1;
          };
          break;
      }

      // сортировать
      rowsArray.sort(compare);

      // ”брать tbody из большого DOM документа дл€ лучшей производительности
      grid.removeChild(tbody);

      // добавить результат в нужном пор€дке в TBODY
      // они автоматически будут убраны со старых мест и вставлены в правильном пор€дке
      for (var i = 0; i < rowsArray.length; i++) {
        tbody.appendChild(rowsArray[i]);
      }

      grid.appendChild(tbody);
    }
    
    if (i == 0) {
    	i = 1;
    }
    else {
    	i = 0;
    }
    
}