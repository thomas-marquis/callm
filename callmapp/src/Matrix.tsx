import React, { useEffect, useRef } from 'react';
import * as d3 from 'd3';

interface MatrixVisualizationProps {
  matrix: number[][];
}

const Matrix: React.FC<MatrixVisualizationProps> = ({ matrix }) => {
  const ref = useRef<SVGSVGElement>(null);

  useEffect(() => {
    if (matrix.length === 0 || !ref.current) return;

    const svg = d3.select(ref.current);
    const cellSize = 50;
    const rows = matrix.length;
    const cols = matrix[0].length;

    const width = cols * cellSize;
    const height = rows * cellSize;

    svg.attr('width', width)
        .attr('height', height);

    const minValue = d3.min(matrix, d => d3.min(d))!;
    const maxValue = d3.max(matrix, d => d3.max(d))!;

    const colorScale = d3.scaleLinear()
    .domain([minValue, maxValue])
    .range(['violet', 'indigo', 'blue', 'green', 'yellow', 'orange', 'red']);

    const groups = svg.selectAll('g')
    .data(matrix)
    .join('g')
    .attr('transform', (d, i) => `translate(0, ${i * cellSize})`);

    groups.selectAll('rect')
    .data(d => d)
    .join('rect')
    .attr('x', (d, i) => i * cellSize)
    .attr('width', cellSize)
    .attr('height', cellSize)
    .style('fill', d => colorScale(d))
    .style('stroke', 'black');

    groups.selectAll('text')
    .data(d => d)
    .join('text')
    .attr('x', (d, i) => i * cellSize + cellSize / 2)
    .attr('y', cellSize / 2)
    .attr('text-anchor', 'middle')
    .attr('dominant-baseline', 'middle')
    .text(d => d.toString());

    // Ajouter la fonctionnalité de zoom et de panoramique
    const zoom = d3.zoom<SVGSVGElement, unknown>()
    .scaleExtent([0.5, 5]) // Limites de zoom
    .on('zoom', (event) => {
        const { transform } = event;
        svg.attr('transform', transform);
    });

    svg.call(zoom);

    svg.on('contextmenu', (event) => {
        event.preventDefault(); // Empêcher le menu contextuel
    });

    svg.call(d3.drag<SVGSVGElement, unknown>()
    .on('start', (event) => {
        if (event.button === 2) { // Bouton droit de la souris
            svg.interrupt();
        }
    })
    .on('drag', (event) => {
        if (event.button === 2) { // Bouton droit de la souris
            const [x, y] = d3.pointer(event);
            zoom.translateBy(svg, x, y);
        }
    }));

  }, [matrix]);

  return <svg ref={ref}></svg>;
};

export default Matrix;
