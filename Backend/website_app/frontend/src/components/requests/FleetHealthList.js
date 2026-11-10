import React, { useEffect, useState } from 'react';
import { Link } from 'react-router-dom';
import requestService from '../../services/requestService';

const FleetHealthList = ({ showAlert }) => {
    const [fleet, setFleet] = useState([]);
    const [loading, setLoading] = useState(true);
    const [carId, setCarId] = useState('');
    const [sending, setSending] = useState(false);

    useEffect(() => {
        const loadFleet = async () => {
            try {
                const data = await requestService.getAllCarsHealth();
                const actualData = Array.isArray(data) ? data : (data.data || []);
                
                const sortedData = actualData.sort((a, b) => {
                    const healthA = (a.overall_health || '').toUpperCase();
                    const healthB = (b.overall_health || '').toUpperCase();
                    
                    if (healthA === 'CRITICAL' && healthB !== 'CRITICAL') return -1;
                    if (healthA !== 'CRITICAL' && healthB === 'CRITICAL') return 1;
                    return new Date(b.timestamp) - new Date(a.timestamp);
                });
                
                setFleet(sortedData);
            } catch (err) {
                console.error("Failed to load fleet health", err);
                if (showAlert) showAlert("danger", "Failed to fetch fleet health data.");
            } finally {
                setLoading(false);
            }
        };
        loadFleet();
    }, [showAlert]);


    const handleActivateDota = async () => {
        if (!carId.trim()) {
            showAlert?.("warning", "Please enter a Car ID.");
            return;
        }

        try {
            setSending(true);
            const response = await requestService.activateDota(carId);
            showAlert?.("success", response.message || `Active DOTA sent for ${carId}`);
            setCarId('');
        } catch (err) {
            console.error(err);
            showAlert?.("danger", err.response?.data?.error || err.message || "Failed to send Active DOTA message");
        } finally {
            setSending(false);
        }
    };

    const getStatusBadge = (health = '') => {
        if (health.toUpperCase() === 'CRITICAL') {
            return (
                <span className="badge bg-danger">
                    <i className="bi bi-exclamation-triangle me-1"></i> Critical
                </span>
            );
        }
        return (
            <span className="badge bg-success">
                <i className="bi bi-check-circle me-1"></i> Healthy
            </span>
        );
    };

    return (
        <div className="fleet-health-list">
            <div className="d-flex justify-content-between flex-wrap flex-md-nowrap align-items-center pt-3 pb-2 mb-3 border-bottom">
                <h1 className="h2">Fleet Health Monitor</h1>
                <span className="badge bg-secondary fs-6">{fleet.length} Total Vehicles</span>
            </div>

            <div className="card mb-4">
                <div className="card-header">
                    <h5 className="mb-0">Active Vehicles</h5>
                </div>
                <div className="card-body p-0">
                    {loading ? (
                        <div className="p-5 text-center">
                            <div className="spinner-border text-primary" role="status">
                                <span className="visually-hidden">Loading...</span>
                            </div>
                        </div>
                    ) : (
                        <div className="table-responsive">
                            <table className="table table-striped table-hover mb-0">
                                <thead>
                                    <tr>
                                        <th className="ps-4">Status</th>
                                        <th>Vehicle ID</th>
                                        <th>Car Type</th>
                                        <th>Last Diagnostic</th>
                                        <th className="text-center">Details</th>
                                    </tr>
                                </thead>
                                <tbody>
                                    {fleet.map((car, idx) => (
                                        <tr key={car.car_id || idx} className="align-middle">
                                            <td className="ps-4">{getStatusBadge(car.overall_health)}</td>
                                            <td className="fw-bold">{car.car_id}</td>
                                            <td className="text-capitalize">{car.car_type}</td>
                                            <td>
                                                {new Date(car.timestamp).toLocaleString('en-GB', {
                                                    day: '2-digit', month: '2-digit', year: 'numeric',
                                                    hour: '2-digit', minute: '2-digit', second: '2-digit'
                                                })}
                                            </td>
                                            <td className="text-center">
                                                <Link
                                                    to={`/diagnostics/history?car_id=${car.car_id}`}
                                                    className="btn btn-outline-info btn-sm"
                                                >
                                                    <i className="bi bi-box-arrow-up-right me-1"></i> View
                                                </Link>
                                            </td>
                                        </tr>
                                    ))}

                                    {fleet.length === 0 && (
                                        <tr>
                                            <td colSpan="5" className="text-center p-4 text-muted">
                                                No diagnostic data recorded in the fleet.
                                            </td>
                                        </tr>
                                    )}
                                </tbody>
                            </table>
                        </div>
                    )}
                </div>
            </div>

            <div className="card mb-4">
                <div className="card-header">
                    <h5 className="mb-0">Activate DOTA</h5>
                </div>
                <div className="card-body">
                    <div className="row g-3 align-items-center">
                        <div className="col-md-8">
                            <input
                                type="text"
                                className="form-control"
                                placeholder="Enter Car ID (e.g., mc180a)"
                                value={carId}
                                onChange={(e) => setCarId(e.target.value)}
                            />
                        </div>
                        <div className="col-md-4">
                            <button
                                className="btn btn-primary w-100"
                                onClick={handleActivateDota}
                                disabled={sending}
                            >
                                <i className="bi bi-broadcast me-2"></i>
                                {sending ? 'Sending...' : 'Send Active DOTA'}
                            </button>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    );
};

export default FleetHealthList;