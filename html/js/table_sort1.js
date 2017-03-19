function TABLESORT(grid) {
var i = 0;
//var grid = document.getElementById('resultTable');

//alert(i);

    grid.onclick = function(e) {
    
    alert(i);
    
      if (e.target.tagName != 'TH') return;

      // ���� TH -- ���������
      sortGrid(e.target.cellIndex, e.target.getAttribute('data-type'));
    };

    function sortGrid(colNum, type) {
      var tbody = grid.getElementsByTagName('tbody')[0];

      // ��������� ������ �� TR
      var rowsArray = [].slice.call(tbody.rows);

      // ���������� ������� ���������, � ����������� �� ����
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

      // �����������
      rowsArray.sort(compare);

      // ������ tbody �� �������� DOM ��������� ��� ������ ������������������
      grid.removeChild(tbody);

      // �������� ��������� � ������ ������� � TBODY
      // ��� ������������� ����� ������ �� ������ ���� � ��������� � ���������� �������
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